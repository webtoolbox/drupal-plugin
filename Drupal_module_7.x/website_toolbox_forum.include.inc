<?php
const WT_API_URL = 'https://api.websitetoolbox.com/dev/api';

function website_toolbox_forum_get_forum_authtoken($user, $type){
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
function website_toolbox_forum_save_authtoken($authtoken, $forumUserid){
    website_toolbox_forum_check_session();
    $_SESSION['forumLoginAuthToken'] = $authtoken;
    website_toolbox_forum_set_forum_cookies('forumLoginUserid', $forumUserid, 0);
}
function website_toolbox_forum_print_image_tag_logout(&$data, $elements){
    $forumUrl = variable_get('forum_url');
    if (!user_is_logged_in() && (isset($_COOKIE['forumLogoutToken']) || isset($_SESSION['forumLoginAuthToken']))) {
        $logout_auth_url = $forumUrl . "/register/logout?authtoken=" . $_COOKIE['forumLogoutToken'];
        $logout_auth_url = preg_replace('#^https?:#i', '', $logout_auth_url);
        $img             = '<img src="' . $logout_auth_url . '" border="0" width="0" height="0" alt="">';
        $data            = preg_replace('@</body[^>]*>@', '$0' . $img, $data, 1);
        website_toolbox_forum_unset_login_session();
        website_toolbox_forum_reset_cookie_on_logout();
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
function website_toolbox_forum_get_userid($userEmail){
    if ($userEmail) {
        $data     = array(
            "email" => $userEmail
        );
        $response = website_toolbox_forum_send_api_request('GET', "/users/", $data);
        if ($response->{'data'}[0]->{'userId'}) {
            echo $response->{'data'}[0]->{'userId'};
            return $response->{'data'}[0]->{'userId'};
        }
    }
}
function website_toolbox_forum_send_api_request($method, $path, $data){
    $url = WT_API_URL . $path;
    if (strtoupper($method) == "GET") {
        $url = sprintf("%s?%s", $url, http_build_query($data));
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
    }
    $response = curl_exec($curl);
    curl_close($curl);
    return json_decode($response);
}
function website_toolbox_forum_set_cookie_on_login($responseArray, $rememberme){
    $persistent = 0;
    if ($rememberme != "") {
        website_toolbox_forum_set_forum_cookies('loginRemember', "checked", $persistent);
        $persistent = 1;
    }
    // Save authentication token into cookie for one day to use into SSO logout.
    if (isset($responseArray['authtoken'])) {
        website_toolbox_forum_set_forum_cookies('forumLogoutToken', $responseArray['authtoken'], $persistent);
    }
    return true;
}
function website_toolbox_forum_check_session()
{
    if (!session_id()) {
        drupal_session_start();
    }
}
function website_toolbox_forum_reset_cookie_on_logout(){
    global $base_path;
    setcookie('forumLogoutToken', 0, time() - 3600, $base_path);
    unset($_COOKIE['forumLogoutToken']);
    setcookie('forumLoginUserid', '', time() - 3600, $base_path);
    unset($_COOKIE['forumLoginUserid']);
    setcookie('loginRemember', '', time() - 3600, $base_path);
    unset($_COOKIE['loginRemember']);
}
function website_toolbox_forum_set_forum_cookies($cname, $cvalue, $persistent){
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
            setcookie($cname, $cvalue, $expiration, $base_path . ";SameSite=None;Secure;", $base_url);
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
function website_toolbox_forum_unset_login_session(){
    unset($_SESSION['forumLoginAuthToken']);
}