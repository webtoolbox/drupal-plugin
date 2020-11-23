<?php
namespace Drupal\website_toolbox_forum;

const WT_API_URL = 'https://api.websitetoolbox.com/v1/api';
function website_toolbox_forum_get_forum_authtoken($user, $type){
    $forumsApiKey = \Drupal::config('node.settings')->get('forum_api_key');
    $forumUrl     = \Drupal::config('node.settings')->get('forum_url');
    if ($forumsApiKey) {
        $username = $user->name;
        $email    = $user->mail;
        $userId   = $user->externalUserid;
        $URL      = $forumUrl . "/register/setauthtoken";
        $request = \Drupal::httpClient()->post($URL, ['verify' => true,'form_params' => ['type' => 'json','apikey' => $forumsApiKey,'user' => $username,'email' =>$email,'externalUserid' => $userId,]])->getBody()->getContents();
        //Check if http/https request could not return any error then filter JSON from response
        if ($request) {
            $response = json_decode($request);
            if ($response && $response->authtoken != "") {
                $authtoken     = $response->authtoken;
                $forumUserid   = $response->userid;
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
    $_SESSION['forumLoginAuthToken'] = $authtoken;
    website_toolbox_forum_set_forum_cookies('forumLoginUserid', $forumUserid, 0);
}
function website_toolbox_forum_get_userid($userEmail){
    if ($userEmail) {
        $userEmailDetail = array(
            'email' => $userEmail
        );
        $url             = WT_API_URL . "/users/";
        $response        = website_toolbox_forum_send_api_request('GET', "/users/", $userEmailDetail);
        if ($response->{'data'}[0]->{'userId'}) {
            return $response->{'data'}[0]->{'userId'};
        }
    }
}
function website_toolbox_forum_send_api_request($method, $requestPath, $requestData){
    $requestUrl = WT_API_URL . $requestPath;
    if (strtoupper($method) == "GET") {
        $requestUrl = sprintf("%s?%s", $requestUrl, http_build_query($requestData));
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $requestUrl);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
        "x-api-key: " . \Drupal::config('node.settings')->get('forum_api_key'),
        'Content-Type: application/json'
    ));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    if (strtoupper($method) == "POST") {
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($requestData));
    } else if (strtoupper($method) == "GET") {
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
function website_toolbox_forum_reset_cookie_on_logout(){
    setcookie('forumLogoutToken', 0, time() - 3600, "/");
    unset($_COOKIE['forumLogoutToken']);
    setcookie('forumLoginUserid', '', time() - 3600, "/");
    unset($_COOKIE['forumLoginUserid']);
    setcookie('loginRemember', '', time() - 3600, "/");
    unset($_COOKIE['loginRemember']);
}
function website_toolbox_forum_set_forum_cookies($cname, $cvalue, $persistent){
    if ($persistent == 1) {
        $expiration = (int) time() + (86400 * 365);
    } else {
        $expiration = 0;
    }
    // Pass param in setcookie function as a array if php version is 7.3 or above, to overcome the waring return on wordpress website.
    if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
        setcookie($cname, $cvalue, $expiration, "/" . " ; SameSite=None; Secure;");
    } else {
        setcookie($cname, $cvalue, $expiration, '/');
    }
}
function website_toolbox_forum_unset_login_session(){
    unset($_SESSION['forumLoginAuthToken']);
}
function website_toolbox_forum_get_domain_name($url){
    //if $url is empty return $url 
    if (empty($url)) {
        return $url;
    }
    $parsedURL = parse_url($url);
    $domain    = isset($parsedURL['host']) ? $parsedURL['host'] : $parsedURL['path'];
    if (preg_match('/(?P<domain>[a-z0-9][a-z0-9\-]{1,63}\.[a-z\.]{2,6})$/i', $domain, $regs)) {
        return $regs['domain'];
    }
}
?>