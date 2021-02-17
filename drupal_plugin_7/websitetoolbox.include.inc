<?php
namespace WebsiteToolboxSSO;
function getForumAuthtoken($user, $type){
    if ($type == 'login') {
        global $user;
    }
    $forumsApiKey = variable_get('forum_api_key');
    $forumUrl     = variable_get('forum_url');
    if ($forumsApiKey) {
        $username = $user->name;
        $email    = $user->mail;
        $userId   = $user->uid;
        $URL      = $forumUrl . "/register/setauthtoken";
        $data     = array(
            'type' => 'json',
            'apikey' => $forumsApiKey,
            'user' => $username,
            'email' => $email,
            'externalUserid' => $userId
            
        );
        $options  = array(
            'type' => 'json',
            'method' => 'POST',
            'data' => drupal_http_build_query($data),
            'timeout' => 15
            
        );
        $pass     = $user->pass;
        if (isset($pass) && $pass != "") {
            $data['pw'] = $pass;
        }
        $result = drupal_http_request($URL, $options);
        //Check if http/https request could not return any error then filter JSON from response
        if ($result) {
            $response = json_decode($result->data);
            if ($response && $response->{'authtoken'} != "") {
                $authtoken     = $response->{'authtoken'};
                $forumUserid   = $response->{'userid'};
                $responseArray = array(
                    'authtoken' => $authtoken,
                    'forumUserid' => $forumUserid
                );
                return $responseArray;
            }
        }
    }
}
function saveAuthToken($authtoken, $forumUserid){
    checkSession();
    $_SESSION['forumLoginAuthToken'] = $authtoken;
    setForumCookies('forumLoginUserid', $forumUserid, 0);
}
function printImageTagLogout(&$data, $elements){
    $forumUrl = variable_get('forum_url');
    if (!user_is_logged_in() && (isset($_COOKIE['forumLogoutToken']) || isset($_SESSION['forumLoginAuthToken']))) {
        $logout_auth_url = $forumUrl . "/register/logout?authtoken=" . $_COOKIE['forumLogoutToken'];
        $logout_auth_url = preg_replace('#^https?:#i', '', $logout_auth_url);
        $img             = '<img src="' . $logout_auth_url . '" border="0" width="0" height="0" alt="">';
        $data            = preg_replace('@</body[^>]*>@', '$0' . $img, $data, 1);
        unsetLoginSession();
        resetCookieOnLogout();
    } else {
        $img  = '';
        $data = preg_replace('@<body[^>]*>@', '$0' . $img, $data, 1);
    }
    if (user_is_logged_in() && (isset($_COOKIE['forumLogoutToken']) || isset($_SESSION['forumLoginAuthToken']))) {
        $loginAuthUrl = $forumUrl . "/register/dologin?authtoken=" . $_SESSION['forumLoginAuthToken'];
        $loginAuthUrl = preg_replace('#^https?:#i', '', $loginAuthUrl);
        $img          = '<img src="' . $loginAuthUrl . '" border="0" width="0" height="0" alt="">';
        $data         = preg_replace('@</body[^>]*>@', '$0' . $img, $data, 1);        
    } else {
        $img  = '';
        $data = preg_replace('@<body[^>]*>@', '$0' . $img, $data, 1);
    }    
    return $data;
}
function getUserDetails($userEmail){
    if ($userEmail) {
        $data     = array(
            "email" => $userEmail
        );
        $response = sendApiRequest('GET', "/users/", $data);
        return $response;
    }
}
function sendApiRequest($method, $path, $data = ''){
    $url = WT_API_URL . $path;
    if (strtoupper($method) == "GET") {
        if ($data != '') {
            $url = sprintf("%s?%s", $url, http_build_query($data));
        }
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
        "x-api-key: " . variable_get('forum_api_key'),
        'Content-Type: application/json'
    ));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    if (strtoupper($method) == "POST") {
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($data));
    } elseif (strtoupper($method) == "GET") {
        curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    } elseif (strtoupper($method) == "DELETE") {
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($data));
    }
    $response = curl_exec($curl);
    curl_close($curl);
    return json_decode($response);
}
function getUserGroupByName($userGroupDetails){
    $UserGroupByName = [];
    $userGroupId = $userGroupDetails->{'data'}[0]->{'userGroups'};
    for ($i = 0; $i < count($userGroupId); $i++) {
        $requestUrl = "/usergroups/" . $userGroupId[$i];
        $response   = sendApiRequest('GET', $requestUrl);
        if ($response->{'defaultGroup'} == false) {
            $userGroupIds                   = $response->{'userGroupId'};
            $UserGroupByName[$userGroupIds] = $response->{'title'};
        }
    }
    return $UserGroupByName;
}
function getUserGroupId($title){
    $getUserGroupList = getGroupList();
    for ($i = 0; $i < count($getUserGroupList['data']); $i++) {
        $userGroupID              = $getUserGroupList['data'][$i]->{"userGroupId"};
        $userGroups[$userGroupID] = $getUserGroupList['data'][$i]->{"title"};
    }
    $userGroupId = array_search($title, $userGroups);
    return $userGroupId;
}
function getGroupList(){
    $requestUrl = "/usergroups";
    $limit      = array(
        "limit" => 100
    );
    $response   = (array) sendApiRequest('GET', $requestUrl, $limit);
    return $response;
}
function getCustomUserGroups(){
    $getUserGroupList = getGroupList();
    for ($i = 0; $i < count($getUserGroupList['data']); $i++) {
        if ($getUserGroupList['data'][$i]->{"defaultGroup"} === false) {
            $userGroupID              = $getUserGroupList['data'][$i]->{"userGroupId"};
            $userGroups[$userGroupID] = $getUserGroupList['data'][$i]->{"title"};
        }
    }
    return $userGroups;
}
function assignDivestUserGroup($userEmail, $username, $userGroupsToAssign = '', $userGroupsToDivest = ''){
    $userDetails = array(
        "emailAddresses" => $userEmail,
        "usernames" => $username
    );
    if (!empty($userGroupsToAssign)) {
        foreach ($userGroupsToAssign as $userGroupsToAssignRoles) {
            $requestUrl = "/usergroups/" . $userGroupsToAssignRoles . "/users";
            sendApiRequest('POST', $requestUrl, $userDetails);
        }
    }
    if (!empty($userGroupsToDivest)) {
        foreach ($userGroupsToDivest as $userGroupsToDivestRoles) {
            $requestUrl = "/usergroups/" . $userGroupsToDivestRoles . "/users";
            sendApiRequest('DELETE', $requestUrl, $userDetails);
        }
    }
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
function checkSession(){
    if (!session_id()) {
        drupal_session_start();
    }
}
function resetCookieOnLogout(){
    global $base_path;
    setcookie('forumLogoutToken', 0, time() - 3600, $base_path);
    unset($_COOKIE['forumLogoutToken']);
    setcookie('forumLoginUserid', '', time() - 3600, $base_path);
    unset($_COOKIE['forumLoginUserid']);
    setcookie('loginRemember', '', time() - 3600, $base_path);
    unset($_COOKIE['loginRemember']);
}
function setForumCookies($cname, $cvalue, $persistent){
    global $base_url;
    global $base_path;
    if ($persistent == 1) {
        $expiration = (int) time() + (86400 * 365);
    } else {
        $expiration = 0;
    }
    setcookie($cname, $cvalue);
    // Pass param in setcookie function as a array if php version is 7.3 or above, to overcome the waring return on wordpress website.
    if (PHP_VERSION_ID < 70300) {
        if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
            setcookie($cname, $cvalue, $expiration, $base_path . " ; SameSite=None; Secure;", $base_url);
        } else {
            setcookie($cname, $cvalue, $expiration, $base_url);
        }
    } else {
        $forumBaseCookieOptions = array(
            'expires' => $expiration,
            'path' => $base_path,
            'domain' => $base_url,
            'httponly' => true
        );
        if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
            $forumBaseCookieOptions['secure']   = true;
            $forumBaseCookieOptions['samesite'] = 'None';
        }
    }
}
function unsetLoginSession(){
    unset($_SESSION['forumLoginAuthToken']);
}
?>
<style>
.form-item-forum-embedded-option{
    margin-top: -20px !important;
    margin-left: 192px !important;
}
.form-item-forum-roles-sync-option{
    margin-top: -20px !important;
    margin-left: 192px !important;
    
}
.form-item-forum-embedded-option .description{
    font-weight: 100;
    margin-left: 0px !important;
}
.form-item-forum-roles-sync-option .description{
    margin-left: 0px !important;
    font-weight: 100;
}
</style>