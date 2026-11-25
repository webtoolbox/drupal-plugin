<?php
namespace Drupal\website_toolbox_forum;
const WT_API_URL = 'https://api.websitetoolbox.com/v1/api';
/* Resolve the API gateway base URL from the configured forum address.
   Beta forums (betaN.websitetoolbox.com) authenticate against their own
   gateway stage (api.websitetoolbox.com/betaN/api); live forums use the
   production stage (v1). */
function getApiBaseUrl(){
    $forumUrl = \Drupal::config('node.settings')->get('forum_url', '');
    $host = parse_url($forumUrl, PHP_URL_HOST);
    if ($host && preg_match('/^beta([0-9]+)\.websitetoolbox\.com$/', $host, $matches)) {
        return 'https://api.websitetoolbox.com/beta' . $matches[1] . '/api';
    }
    return WT_API_URL;
}
function getForumAuthtoken($user, $type){
    $forumsApiKey = \Drupal::config('node.settings')->get('forum_api_key');
    $forumUrl     = \Drupal::config('node.settings')->get('forum_url');
    if ($forumsApiKey) {
        $username = $user->name;
        $email    = $user->mail;
        $userId   = $user->externalUserid;
        $URL      = $forumUrl . "/register/setauthtoken";
        $request = \Drupal::httpClient()->post($URL, [
            'verify' => true,
            'form_params' => [
                'type' => 'json',
                'apikey' => $forumsApiKey,
                'user' => $username,
                'email' =>$email,
                'externalUserid' => $userId,]])->getBody()->getContents();
        //Check if http/https request could not return any error then filter JSON from response
        if ($request) {
            $response = json_decode($request);
             if ($response && isset($response->authtoken) && $response->authtoken !== "") {
                $authtoken     = $response->authtoken;
                $forumUserid   = $response->userid ?? '';
                $responseArray = array(
                    'authtoken'   => $authtoken,
                    'forumUserid' => $forumUserid
                );
                return $responseArray;
             }
          }
        }
    }
function saveAuthToken($authtoken, $forumUserid){
    $_SESSION['forumLoginAuthToken'] = $authtoken;
    setForumCookies('forumLoginUserid', $forumUserid, 0);
}
function getUserDetails($userEmail){
    if ($userEmail) {
        $userEmailDetail = array('email' => $userEmail);
        $url = getApiBaseUrl()."/users/";
        $response = sendApiRequest('GET', "/users/", $userEmailDetail);
        return $response;
    }
} 
function getUserGroupsByTitle(){
    $getUserGroupList = getGroupList();
    $userGroups = [];
    $groupListData = $getUserGroupList['data'] ?? [];
    if (!is_array($groupListData)) {
        return $userGroups;
    }
    for($i=0; $i< count($groupListData);$i++){
        $groupTitle = $groupListData[$i]->{"title"} ?? '';
        if($groupTitle != ''){
            $userGroups[strtolower($groupTitle)] = $groupListData[$i]->{"userGroupId"} ?? '';
        }
    }
    return $userGroups;
}
function getUserGroupIdByRole($roleId, $roleTitle = '', $createGroup = TRUE, $userTitle = ''){
    $roleTitle = ($roleTitle != '') ? $roleTitle : $roleId;
    $userGroups = getUserGroupsByTitle();
    if(isset($userGroups[strtolower($roleId)])){
        return $userGroups[strtolower($roleId)];
    }
    if(isset($userGroups[strtolower($roleTitle)])){
        return $userGroups[strtolower($roleTitle)];
    }
    if(!$createGroup){
        return FALSE;
    }
    $groupDetails = array("title" => $roleTitle);
    if($userTitle != ''){
        $groupDetails["usertitle"] = $userTitle;
    }
    $response = sendApiRequest('POST', "/usergroups", $groupDetails);
    return $response->{"userGroupId"} ?? FALSE;
}
function getUserGroupIdsForRoles($roles){
    $userGroups = [];
    $roleStorage = \Drupal::entityTypeManager()->getStorage('user_role');
    foreach ($roles as $roleId) {
        $role = $roleStorage->load($roleId);
        $roleTitle = $role ? $role->label() : $roleId;
        $userGroupId = getUserGroupIdByRole($roleId, $roleTitle);
        if($userGroupId){
            $userGroups[$userGroupId] = $roleTitle;
        }
    }
    return $userGroups;
}
function getGroupList(){
    $requestUrl = "/usergroups";
    $limit = array("limit"=>100); 
    $response = (array)sendApiRequest('GET',$requestUrl,$limit); 
    return $response;
}
function assignDivestUserGroup($userEmail, $username, $userGroupsToAssign ='', $userGroupsToDivest = '' ){ 
    $userDetails = array("emailAddresses"=>$userEmail,"usernames"=>$username);
    if(!empty($userGroupsToAssign)){
      foreach ($userGroupsToAssign as $userGroupsToAssignRoles) {
        $requestUrl = "/usergroups/".$userGroupsToAssignRoles."/users";
        sendApiRequest('POST',$requestUrl,$userDetails); 
      }
    }
    if(!empty($userGroupsToDivest)){
      foreach ($userGroupsToDivest as $userGroupsToDivestRoles) {
         $requestUrl = "/usergroups/".$userGroupsToDivestRoles."/users";
         sendApiRequest('DELETE',$requestUrl,$userDetails); 
      }
    }
}
function sendApiRequest($method, $requestPath, $requestData = ''){
    $requestUrl = getApiBaseUrl() . $requestPath;
     if (strtoupper($method) == "GET") {
        if($requestData != ''){
            $requestUrl = sprintf("%s?%s", $requestUrl, http_build_query($requestData));
        }
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $requestUrl);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
        "x-api-key: " .  \Drupal::config('node.settings')->get('forum_api_key'),
        'Content-Type: application/json'
    ));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    if (strtoupper($method) == "POST") {
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($requestData));
    } else if (strtoupper($method) == "GET") {
        curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    } else if(strtoupper($method) == "DELETE"){
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($requestData));
    }
    $response = curl_exec($curl);
    return json_decode($response);
}
function setCookieOnLogin($responseArray, $rememberme){
    $persistent = 0;
    if ($rememberme != "") {
        setForumCookies('loginRemember', "checked", $persistent);
        $persistent = 1;
    }
    // Save authentication token into cookie for one day to use into SSO logout.
    if (isset($responseArray['authtoken'])) {
        setForumCookies('forumLogoutToken', $responseArray['authtoken'], $persistent);
    }
    return true;
}
function resetCookieOnLogout(){
    // Expire the cookies in the browser only. Do NOT unset $_COOKIE here:
    // hook_page_bottom runs before the menu/node render in the same request,
    // and wiping the superglobal made later hooks (link_alter, node_view)
    // miss the authtoken.
    setcookie('forumLogoutToken', 0, time() - 3600, "/");
    setcookie('forumLoginUserid', '', time() - 3600, "/");
    setcookie('loginRemember', '', time() - 3600, "/");
}
function setForumCookies($cname, $cvalue, $persistent){
    // The forum API can answer without a userid (an unknown user still gets an
    // authtoken), so coerce the value: setcookie() rejects NULL on PHP 8.
    $cvalue = (string) $cvalue;
    if ($persistent == 1) {
        $expiration = (int) time() + (86400 * 365);
    } else {
        $expiration = 0;
    }
// Use the options-array form (PHP 7.3+) so SameSite/Secure are proper cookie
// options; stuffing them into the path argument is a ValueError fatal on PHP 8.
    if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
        setcookie($cname, $cvalue, ['expires' => $expiration, 'path' => '/', 'samesite' => 'None', 'secure' => TRUE]);
    } else {
        setcookie($cname, $cvalue, ['expires' => $expiration, 'path' => '/']);
    }
}
function unsetLoginSession(){
    unset($_SESSION['forumLoginAuthToken']);
}
function getDomainName($url){
  //if $url is empty return $url 
    if(empty($url)){
        return $url;
    }
    $parsedURL = parse_url($url);
    $domain = isset($parsedURL['host']) ? $parsedURL['host'] : ($parsedURL['path'] ?? '');
    if (preg_match('/(?P<domain>[a-z0-9][a-z0-9\-]{1,63}\.[a-z\.]{2,6})$/i', $domain, $regs)) {
        return $regs['domain'];
    }
}
