<?php  function setCookieOnLogin($responseArray, $rememberme) {
        $persistent = 0;
        if($rememberme != "") {
            setWtbCookies('wtbLoginRemember', "checked", $persistent);
            $persistent = 1;
        }
        // Save authentication token into cookie for one day to use into SSO logout.
        if(isset($responseArray['authtoken'])){
            setWtbCookies('wtbLogoutToken', $responseArray['authtoken'], $persistent);
        }
        // Save access token into cookie for one day to use into SSO logout from Chat Room.


        return true;
    }

    function checkSession(){
        if(!session_id()){
            drupal_session_start();
        }
    }
    function setWtbCookies($cname, $cvalue, $persistent) {
        if($persistent == 1) {
            $expiration = (int) time() + (86400 * 365);
        } else {
            $expiration = 0;
        }
        setcookie($cname, $cvalue, $wtbBaseCookieOptions);
        // Pass param in setcookie function as a array if php version is 7.3 or above, to overcome the waring return on wordpress website.
        if(PHP_VERSION_ID < 70300) {
            if(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
                setcookie($cname, $cvalue, $expiration, COOKIEPATH." ; SameSite=None; Secure;", COOKIE_DOMAIN);
            } else {
                setcookie($cname, $cvalue, $expiration, COOKIEPATH, COOKIE_DOMAIN);
            }
        } else {
            $wtbBaseCookieOptions = [
                'expires' => $expiration,
                'path' => COOKIEPATH,
                'domain' => COOKIE_DOMAIN,
                'httponly' => true,
            ];
            if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off') {
                $wtbBaseCookieOptions['secure'] = true;
                $wtbBaseCookieOptions['samesite'] = 'None';
            }
        }
    }

    function resetCookieOnLogout() {
        unset($_COOKIE['wtbLogoutToken']);
        setcookie('wtbLogoutToken', 0, time() - 3600, $base_path);

        unset($_COOKIE['wtbLoginUserid']);
        setcookie('wtbLoginUserid', '', time() - 3600, $base_path);
        unset($_COOKIE['wtbLoginRemember']);
        setcookie('wtbLoginRemember', '', time() - 3600, $base_path);
    }
    function unsetLoginSession() {
        unset($_SESSION['wtbLoginAuthToken']);

    }?>