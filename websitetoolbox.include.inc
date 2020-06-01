<?php
namespace WebsiteToolboxSSO {
    function getForumAuthtoken($user, $type)
    {
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
    function saveAuthToken($authtoken, $access_token, $forumUserid)
    {
        checkSession();
        $_SESSION['forumLoginAuthToken'] = $authtoken;
        setForumCookies('forumLoginUserid', $forumUserid, 0);
    }
    function printImageTagLogout(&$data, $elements)
    {
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
        return $data;
    }
    function getUserid($userEmail)
    {
        if ($userEmail) {
            $data     = array(
                "email" => $userEmail
            );
            $response = sendApiRequest('GET', "/users/", $data);
            if ($response->{data}[0]->{userId}) {
                return $response->{data}[0]->{userId};
            }
        }
    }
    function sendApiRequest($method, $path, $data)
    {
        $url = API_URL . $path;
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
        } else if (strtoupper($method) == "GET") {
            curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        }
        $response = curl_exec($curl);
        curl_close($curl);
        return json_decode($response);
    }
    
    function setCookieOnLogin($responseArray, $rememberme)
    {
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
    function checkSession()
    {
        if (!session_id()) {
            drupal_session_start();
        }
    }
    
    function resetCookieOnLogout()
    {
        setcookie('forumLogoutToken', 0, time() - 3600, $base_path);
        setcookie('forumLoginUserid', '', time() - 3600, $base_path);
        setcookie('loginRemember', '', time() - 3600, $base_path);
    }
    
    function setForumCookies($cname, $cvalue, $persistent)
    {
        if ($persistent == 1) {
            $expiration = (int) time() + (86400 * 365);
        } else {
            $expiration = 0;
        }
        setcookie($cname, $cvalue, $forumBaseCookieOptions);
        // Pass param in setcookie function as a array if php version is 7.3 or above, to overcome the waring return on wordpress website.
        if (PHP_VERSION_ID < 70300) {
            if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
                setcookie($cname, $cvalue, $expiration, COOKIEPATH . " ; SameSite=None; Secure;", COOKIE_DOMAIN);
            } else {
                setcookie($cname, $cvalue, $expiration, COOKIEPATH, COOKIE_DOMAIN);
            }
        } else {
             $forumBaseCookieOptions = [
                'expires' => $expiration,
                'path' => COOKIEPATH,
                'domain' => COOKIE_DOMAIN,
                'httponly' => true,
            ];
            if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
                $forumBaseCookieOptions['secure']   = true;
                $forumBaseCookieOptions['samesite'] = 'None';
            }
        }
    }
    
    function unsetLoginSession()
    {
        unset($_SESSION['forumLoginAuthToken']);
    }
}

?>