<?php
namespace Drupal\website_toolbox_forum;
const WT_API_URL = 'https://api.websitetoolbox.com/v1/api';
function getForumAuthtoken($user, $type){
    $forumsApiKey = \Drupal::config('node.settings')->get('forum_api_key');
    $forumUrl     = \Drupal::config('node.settings')->get('forum_url');
    if ($forumsApiKey) {
        $username   = $user->name;
        $email      = $user->mail;
        $userId     = $user->externalUserid;
        $URL        = $forumUrl . "/register/setauthtoken";
        $request    = \Drupal::httpClient()->post($URL, [
            'verify'        => true,
            'form_params'   => [
                'type'          => 'json',
                'apikey'        => $forumsApiKey,
                'user'          => $username,
                'email'         =>$email,
                'externalUserid'=> $userId,
                ]
        ])->getBody()->getContents();
        //Check if http/https request could not return any error then filter JSON from response
        if ($request) {
            $response = json_decode($request);
            if ($response && $response->authtoken != "") {
                $authtoken     = $response->authtoken;
                $forumUserid   = $response->userid;
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
        $userEmailDetail = array(
            'email' => $userEmail
        );
        $url = WT_API_URL."/users/";
        $response = sendApiRequest('GET', "/users/", $userEmailDetail);
        return $response;
    }
} 
function getUserGroupByName($userGroupDetails){
    $UserGroupByName = [];
    $userGroupId = $userGroupDetails->{'data'}[0]->{'userGroups'};
    for($i=0; $i<count($userGroupId);$i++){
        $requestUrl = "/usergroups/".$userGroupId[$i];
        $response   = sendApiRequest('GET',$requestUrl); 
        if($response->{'defaultGroup'} == false)
        {   $userGroupIds                   = $response->{'userGroupId'}; 
            $UserGroupByName[$userGroupIds] = $response->{'title'};
        }
    }
    return $UserGroupByName;
}
function getUserGroupId($title){
    $getUserGroupList = getGroupList();
    for($i=0; $i< count($getUserGroupList['data']);$i++){
        $userGroupID                =  $getUserGroupList['data'][$i]->{"userGroupId"};
        $userGroups[$userGroupID]   = $getUserGroupList['data'][$i]->{"title"};
    }  
    $userGroupId = array_search($title,$userGroups);
    return $userGroupId;
}
function getGroupList(){
    $requestUrl = "/usergroups";
    $limit      = array("limit"=>100); 
    $response   = (array)sendApiRequest('GET',$requestUrl,$limit); 
    return $response;
}
function getCustomUserGroups(){
    $getUserGroupList = getGroupList();
    for($i=0; $i< count($getUserGroupList['data']);$i++){
        if($getUserGroupList['data'][$i]->{"defaultGroup"} === false){
            $userGroupID                =  $getUserGroupList['data'][$i]->{"userGroupId"};
            $userGroups[$userGroupID]   =  $getUserGroupList['data'][$i]->{"title"};
        }
    }  
    return $userGroups;
 }
function assignDivestUserGroup($userEmail, $username, $userGroupsToAssign ='', $userGroupsToDivest = '' ){
    $userDetails = array(
        "emailAddresses"    =>$userEmail,
        "usernames"         =>$username
    );
    if(!empty($userGroupsToAssign)){
      for($i=0;$i<count($userGroupsToAssign);$i++){ 
         $requestUrl = "/usergroups/".$userGroupsToAssign[$i]."/users";
         sendApiRequest('POST',$requestUrl,$userDetails); 
      }
    }
    if(!empty($userGroupsToDivest)){
      for($j=0;$j<count($userGroupsToDivest);$j++){ 
         echo $requestUrl = "/usergroups/".$userGroupsToDivest[$j]."/users";
         sendApiRequest('DELETE',$requestUrl,$userDetails); 
      }
    }
}
function sendApiRequest($method, $requestPath, $requestData = ''){
    $requestUrl = WT_API_URL . $requestPath;
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
    curl_close($curl);
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
    setcookie('forumLogoutToken', 0, time() - 3600, "/");
    unset($_COOKIE['forumLogoutToken']);
    setcookie('forumLoginUserid', '', time() - 3600, "/");
    unset($_COOKIE['forumLoginUserid']);
    setcookie('loginRemember', '', time() - 3600, "/");
    unset($_COOKIE['loginRemember']);
}
function setForumCookies($cname, $cvalue, $persistent){
    if ($persistent == 1) {
        $expiration = (int) time() + (86400 * 365);
    } else {
        $expiration = 0;
    }
// Pass param in setcookie function as a array if php version is 7.3 or above, to overcome the waring return on wordpress website.
    if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
        setcookie($cname, $cvalue, $expiration, "/" . " ; SameSite=None; Secure;");
    } else {
        setcookie($cname, $cvalue, $expiration,'/');
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
    $domain = isset($parsedURL['host']) ? $parsedURL['host'] : $parsedURL['path'];
    if (preg_match('/(?P<domain>[a-z0-9][a-z0-9\-]{1,63}\.[a-z\.]{2,6})$/i', $domain, $regs)) {
        return $regs['domain'];
    }
}
?>
<style>
.js-form-item-forum-embedded-option, .js-form-item-forum-roles-sync-option{
    margin-top: -15px !important;
    margin-left: 191px !important;
}
.classLabel{
    margin-top:20px;
    margin-bottom:20px;
}
.iconHelp {
    margin-top: -20px;
    margin-left: 125px;
}
 
</style>