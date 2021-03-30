<?php
function websitetoolbox_get_forum_authtoken($user, $type){
    if ($type == 'login') {
        global $user;
    }
    $forumsApiKey = variable_get('websitetoolbox_forum_api_key');
    $forumUrl     = variable_get('websitetoolbox_forum_url');
    if ($forumsApiKey) {
        $username = $user->name;
        $email    = $user->mail;
        $userId   = $user->uid;
        $URL      = $forumUrl . "/register/setauthtoken";
        $data     = array(
            'type'          => 'json',
            'apikey'        => $forumsApiKey,
            'user'          => $username,
            'email'         => $email,
            'externalUserid'=> $userId
            
        );
        $options  = array(
            'type'      => 'json',
            'method'    => 'POST',
            'data'      => drupal_http_build_query($data),
            'timeout'   => 15
            
        );
        $pass           = $user->pass;
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
                    'authtoken'     => $authtoken,
                    'forumUserid'   => $forumUserid
                );
                return $responseArray;
            }
        }
    }
}
function websitetoolbox_save_authtoken($authtoken, $forumUserid){
    websitetoolbox_check_session();
    $_SESSION['websitetoolboxLoginAuthToken'] = $authtoken;
    websitetoolbox_set_forum_cookies('websitetoolboxLoginUserid', $forumUserid, 0);
}
function websitetoolbox_logout_image_tag(&$data, $elements){
    $forumUrl = variable_get('websitetoolbox_forum_url');
    if (!user_is_logged_in() && (isset($_COOKIE['websitetoolboxLogoutToken']) || isset($_SESSION['websitetoolboxLoginAuthToken']))) {
        $logout_auth_url = $forumUrl . "/register/logout?authtoken=" . $_COOKIE['websitetoolboxLogoutToken'];
        $logout_auth_url = preg_replace('#^https?:#i', '', $logout_auth_url);
        $img             = '<img src="' . $logout_auth_url . '" border="0" width="0" height="0" alt="">';
        $data            = preg_replace('@</body[^>]*>@', '$0' . $img, $data, 1);
        websitetoolbox_unset_login_session();
        websitetoolbox_reset_cookie_on_logout();
    } else {
        $img  = '';
        $data = preg_replace('@<body[^>]*>@', '$0' . $img, $data, 1);
    }
    if (user_is_logged_in() && (isset($_COOKIE['websitetoolboxLogoutToken']) || isset($_SESSION['websitetoolboxLoginAuthToken']))) {
        $loginAuthUrl = $forumUrl . "/register/dologin?authtoken=" . $_SESSION['websitetoolboxLoginAuthToken'];
        $loginAuthUrl = preg_replace('#^https?:#i', '', $loginAuthUrl);
        $img          = '<img src="' . $loginAuthUrl . '" border="0" width="0" height="0" alt="">';
        $data         = preg_replace('@</body[^>]*>@', '$0' . $img, $data, 1);        
    } else {
        $img  = '';
        $data = preg_replace('@<body[^>]*>@', '$0' . $img, $data, 1);
    }    
    return $data;
}
function websitetoolbox_get_user_details($userEmail){
    if ($userEmail) {
        $data     = array(
            "email" => $userEmail
        );
        $response = websitetoolbox_send_api_request('GET', "/users/", $data);
        return $response;
    }
}
function websitetoolbox_send_api_request($method, $path, $data = ''){
    $url = WT_API_URL . $path;
    if (strtoupper($method) == "GET") {
        if ($data != '') {
            $url = sprintf("%s?%s", $url, http_build_query($data));
        }
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
        "x-api-key: " . variable_get('websitetoolbox_forum_api_key'),
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
function websitetoolbox_user_group_by_name($userGroupDetails){
    $UserGroupByName = [];
    $userGroupId = $userGroupDetails->{'data'}[0]->{'userGroups'};
    for ($i = 0; $i < count($userGroupId); $i++) {
        $requestUrl = "/usergroups/" . $userGroupId[$i];
        $response   = websitetoolbox_send_api_request('GET', $requestUrl);
        if ($response->{'defaultGroup'} == false) {
            $userGroupIds                   = $response->{'userGroupId'};
            $UserGroupByName[$userGroupIds] = $response->{'title'};
        }
    }
    return $UserGroupByName;
}
function websitetoolbox_user_group_id($title){ 
    $getUserGroupList = websitetoolbox_get_group_list(); 
    for ($i = 0; $i < count($getUserGroupList['data']); $i++) {
        $userGroupID              = $getUserGroupList['data'][$i]->{"userGroupId"};
        $userGroups[$userGroupID] = $getUserGroupList['data'][$i]->{"title"}; 
    }
    $userGroupId = array_search($title, $userGroups);
    return $userGroupId;
}
function websitetoolbox_get_group_list(){
    $requestUrl = "/usergroups";
    $limit      = array(
        "limit" => 100
    );
    $response   = (array) websitetoolbox_send_api_request('GET', $requestUrl, $limit);
    return $response;
}
function websitetoolbox_get_custom_user_group(){
    $getUserGroupList = websitetoolbox_get_group_list();
    for ($i = 0; $i < count($getUserGroupList['data']); $i++) {
        if ($getUserGroupList['data'][$i]->{"defaultGroup"} === false) {
            $userGroupID              = $getUserGroupList['data'][$i]->{"userGroupId"};
            $userGroups[$userGroupID] = $getUserGroupList['data'][$i]->{"title"};
        }
    }
    return $userGroups;
}
function websitetoolbox_assign_divest_user_group($userEmail, $username, $userGroupsToAssign = '', $userGroupsToDivest = ''){
    $userDetails = array(
        "emailAddresses" => $userEmail,
        "usernames" => $username
    );
    if (!empty($userGroupsToAssign)) {
        foreach ($userGroupsToAssign as $userGroupsToAssignRoles) {
            $requestUrl = "/usergroups/" . $userGroupsToAssignRoles . "/users";
            websitetoolbox_send_api_request('POST', $requestUrl, $userDetails);
        }
    }
    if (!empty($userGroupsToDivest)) {
        foreach ($userGroupsToDivest as $userGroupsToDivestRoles) {
            $requestUrl = "/usergroups/" . $userGroupsToDivestRoles . "/users";
            websitetoolbox_send_api_request('DELETE', $requestUrl, $userDetails);
        }
    }
}
function websitetoolbox_set_cookie_on_login($responseArray, $rememberme){
    $persistent = 0;
    if ($rememberme != "") {
        websitetoolbox_set_forum_cookies('websitetoolboxLoginRemember', "checked", $persistent);
        $persistent = 1;
    }
    // Save authentication token into cookie for one day to use into SSO logout.
    if (isset($responseArray['authtoken'])) {
        websitetoolbox_set_forum_cookies('websitetoolboxLogoutToken', $responseArray['authtoken'], $persistent);
    }
    return true;
}
function websitetoolbox_check_session(){
    if (!session_id()) {
        drupal_session_start();
    }
}
function websitetoolbox_reset_cookie_on_logout(){
    global $base_path;
    setcookie('websitetoolboxLogoutToken', 0, time() - 3600, $base_path);
    unset($_COOKIE['websitetoolboxLogoutToken']);
    setcookie('websitetoolboxLoginUserid', '', time() - 3600, $base_path);
    unset($_COOKIE['websitetoolboxLoginUserid']);
    setcookie('websitetoolboxLoginRemember', '', time() - 3600, $base_path);
    unset($_COOKIE['websitetoolboxLoginRemember']);
}
function websitetoolbox_set_forum_cookies($cname, $cvalue, $persistent){
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
            'expires'   => $expiration,
            'path'      => $base_path,
            'domain'    => $base_url,
            'httponly'  => true
        );
        if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
            $forumBaseCookieOptions['secure']   = true;
            $forumBaseCookieOptions['samesite'] = 'None';
        }
    }
}
function websitetoolbox_unset_login_session(){
    unset($_SESSION['websitetoolboxLoginAuthToken']);
}
?>
<style>
.form-item-websitetoolbox-embedded-option{
    margin-top: -20px !important;
    margin-left: 192px !important;
}
.form-item-websitetoolbox-roles-sync-option{
    margin-top: -20px !important;
    margin-left: 192px !important;
    
}
.form-item-websitetoolbox-embedded-option .description{
    font-weight: 100;
    margin-left: 0px !important;
}
.form-item-websitetoolbox-roles-sync-option .description{
    margin-left: 0px !important;
    font-weight: 100;
}
</style>