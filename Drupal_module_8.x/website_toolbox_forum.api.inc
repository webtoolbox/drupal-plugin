<?php
namespace Drupal\website_toolbox_forum;
const WT_API_URL = 'https://api.websitetoolbox.com/v1/api';
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
          'form_params' => ['type' => 'json',
          'apikey' => $forumsApiKey,
          'user' => $username,
          'email' => $email,
          'externalUserid' => $userId,]
          ])->getBody()->getContents();
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
function saveAuthToken($authtoken, $forumUserid){
    $_SESSION['forumLoginAuthToken'] = $authtoken;
    setForumCookies('forumLoginUserid', $forumUserid, 0);
}
function getUserid($userEmail){
    if ($userEmail) {
        $url = WT_API_URL . "/users/";
        $request = \Drupal::httpClient()->get($url, ['verify' => true,
        'form_params' => [ 'email' => $userEmail,],
        'headers' => ['x-api-key' =>\Drupal::config('node.settings')->get('forum_api_key'),
        'Content-type' => 'application/json',],
        ])->getBody()->getContents();
        $response = json_decode($request);
        if ($response->data[0]->userId) {
           return $response->data[0]->userId;
        }
   }
}
function updateUserDetails ($userDetails,$url,$method){
    $requestUrl = WT_API_URL.$url;
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $requestUrl);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
      "x-api-key: " . \Drupal::config('node.settings')->get('forum_api_key'),
      'Content-Type: application/json'
    ));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    if (strtoupper($method) == "POST") {
      curl_setopt($curl, CURLOPT_POST, true);
      curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($userDetails));
    } else if (strtoupper($method) == "GET") {
      curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
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
?>
