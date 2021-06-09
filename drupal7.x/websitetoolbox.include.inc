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
        $pass           = $user->pass;
        if (isset($pass) && $pass != "") {
            $data['pw'] = $pass;
        }
        $data   = drupal_http_build_query($data);
        $result = websitetoolbox_send_api_request('POST', $URL, $data);
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
        $url = WT_API_URL . "/users/";
        if ($data != '') {
            $url = sprintf("%s?%s", $url, http_build_query($data));
        }
        $response = websitetoolbox_send_api_request('GET', $url);        
        return $response;
    }
}
function websitetoolbox_send_api_request($method, $url, $data = ''){
    $options          = array(
        'type'          => 'json',
        'method'        => $method,
        'data'          => $data,
        'timeout'       => 15,
        'headers'       => array(
        "x-api-key"     => variable_get('websitetoolbox_forum_api_key'),
        'Content-Type'  => 'application/json'
        )
    );
    $result         = drupal_http_request($url, $options);  
    $response       = json_decode($result->{'data'});
    return $response;
}
function websitetoolbox_user_group_by_name($userGroupDetails){
    $UserGroupByName = [];
    $userGroupId = $userGroupDetails->{'data'}[0]->{'userGroups'};
    for ($i = 0; $i < count($userGroupId); $i++) {    
        $requestUrl = "/usergroups/" . $userGroupId[$i];    
        $url        = WT_API_URL . $requestUrl;
        $result     = websitetoolbox_send_api_request('GET', $url);
        $response   = json_decode($result->{'data'});  
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
    $url        = WT_API_URL . $requestUrl;  
    $response   = (array) websitetoolbox_send_api_request('GET', $url);
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
        "emailAddresses"=> $userEmail,
        "usernames"     => $username
    );
    $userDetails = json_encode($userDetails);
    if (!empty($userGroupsToAssign)) {
        foreach ($userGroupsToAssign as $userGroupsToAssignRoles) {
            $requestUrl = WT_API_URL."/usergroups/" . $userGroupsToAssignRoles . "/users";
            websitetoolbox_send_api_request('POST', $requestUrl, $userDetails);
        }
    }
    if (!empty($userGroupsToDivest)) {
        foreach ($userGroupsToDivest as $userGroupsToDivestRoles) {
            $requestUrl = WT_API_URL."/usergroups/" . $userGroupsToDivestRoles . "/users";
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
    $expiration = (int) time() + (86400 * 365);
    setcookie('websitetoolboxLogoutToken', '',$expiration,"/");
    unset($_COOKIE['websitetoolboxLogoutToken']);
    setcookie('websitetoolboxLoginUserid', '',$expiration,"/");
    unset($_COOKIE['websitetoolboxLoginUserid']);
    setcookie('websitetoolboxLoginRemember', '',$expiration,"/");
    unset($_COOKIE['websitetoolboxLoginRemember']);
}
function websitetoolbox_set_forum_cookies($cname, $cvalue, $persistent){
    global $base_url;
    global $base_path;
    $expiration = (int) time() + (86400 * 365);
    setcookie($cname, $cvalue, $expiration, '/'); 
}
function websitetoolbox_unset_login_session(){
    unset($_SESSION['websitetoolboxLoginAuthToken']);
}
