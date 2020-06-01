<?php /*submit username and password for forum configuation */
    /*get authtoken, save authtoken, create cookies after user logged in*/
namespace forumAdminConfigeSettingPage {
    /*create module admin configuration form*/
    function adminConfigPageLayout()
    {
        $form                   = array();
        $html                   = '<h2>Forums by Website Toolbox</h2><br /><b>Please log in to your Website Toolbox account.</b><p>Not a Website Toolbox Forum owner? <a href="https://www.websitetoolbox.com/wordpress" target="_blank">Create a Forum Now!</a></p><p>Please <a href="https://www.websitetoolbox.com/contact?subject=WordPress+Plugin+Setup+Help" target="_blank">Contact Customer Support</a> if you need help getting set up.</p>';
        $form['display_html'][] = array(
            '#type' => 'markup',
            '#markup' => $html
        );
        if (isset($_SESSION['flag']) && ($_SESSION['flag'] == 1)) {
            $html_error             = '<div class="messages error">Please provide correct username and password</div>';
            $form['display_html'][] = array(
                '#type' => 'markup',
                '#markup' => $html_error
            );
            
        }
        if ((variable_get('forum_username') == '') || $_GET['forumSettingOption'] == 1) {
            $form['forum_username'] = array(
                '#type' => 'textfield',
                '#title' => t('Website Toolbox Username'),
                '#default_value' => variable_get('forum_username', ""),
                '#size' => 60,
                '#maxlength' => 250,
                '#required' => TRUE
            );
            $form['forum_password'] = array(
                '#type' => 'password',
                '#title' => t('Website Toolbox Password'),
                '#default_value' => variable_get('forum_password', ""),
                '#size' => 60,
                '#maxlength' => 250,
                '#required' => TRUE
            );
            $html_hidden            = "<input type='hidden' name='embeddedValue' value='" . variable_get('forum_embedded_option') . "'>";
            $form['display_html'][] = array(
                '#type' => 'markup',
                '#markup' => $html_hidden
            );
        } else {
            $html1                  = "<div><b>Website Toolbox Username :  </b>" . variable_get('forum_username') . "&nbsp;&nbsp;&nbsp;     <a href='?q=" . $_GET[q] . "&forumSettingOption=1'>Change</a></div>";
            $form['display_html'][] = array(
                '#type' => 'markup',
                '#markup' => $html1
            );
            $html2                  = '<br /><div>Enable this option to have your forum embedded in a page of your website.Disable this option to have your forum load in a full-sized window. You can use the Layout section in your Website Toolbox account to <a href="https://www.websitetoolbox.com/support/making-your-forum-layout-match-your-website-148" target="_blank">customize your forum layout to match your website</a> or <a href="https://www.websitetoolbox.com/contact?subject=Customize+Forum+Layout" target="_blank">contact Website Toolbox support to customize it for you</a></div><br />';
            $form['display_html'][] = array(
                '#type' => 'markup',
                '#markup' => $html2
            );
        }
        if ($_GET['forumSettingOption'] != 1) {
            $form['forum_embedded_option'] = array(
                '#type' => 'checkbox',
                '#title' => t('Embedded'),
                '#default_value' => variable_get('forum_embedded_option'),
                '#size' => 60,
                '#maxlength' => 250,
                '#required' => FALSE
            );
        }
        
        $form['#submit'][] = 'forumAdminConfigeSettingPage\adminConfigPageSubmit';
        return system_settings_form($form);
    }
    /*create module admin configuration form*/
    /*submit username and password for forum configuation */
    function adminConfigPageSubmit($formId, &$formState)
    {
        if ($_POST['forum_username'] != '') {
            $forumUsername = $_POST['forum_username'];
        } else {
            $forumUsername = variable_get('forum_username');
        }
        if ($_POST['forum_password'] != '') {
            $forumPassword = $_POST['forum_password'];
        } else {
            $forumPassword = variable_get('forum_password');
        }
        $forumEmbeddedOption = $_POST['forum_embedded_option'];
        if (isset($forumEmbeddedOption)) {
            if ($forumEmbeddedOption == 1) {
                $forumEmbeddedOption = 1;
            } else {
                $forumEmbeddedOption = 0;
            }
        } else {
            $forumEmbeddedOption = $_POST['embeddedValue'];
        }
        checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption);
        drupal_static_reset();
        drupal_flush_all_caches();
    }
    
    function checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption)
    {
        $result = getForumDetails($forumUsername, $forumPassword, $forumEmbeddedOption);
        if ($result) {
            $response = json_decode($result->data);
            if ($result->code != 200 || $response->{'errorMessage'}) {
                if ($result->code == 403) {
                    $ip           = file_get_contents('https://api.ipify.org');
                    $errorMessage = "Your IP address ($ip) is blocked. Please <a href='https://www.websitetoolbox.com/contact?subject=WordPress+IP($ip)+Blocked' target='_blank'>Contact Website Toolbox</a>.";
                } elseif ($result->code != 200) {
                    $errorMessage = $response->{'errorMessage'};
                } else {
                    #show error message to the user.
                    $errorMessage = $response->{'errorMessage'};
                    $errorMessage = preg_replace('#^Error:#', '', $errorMessage);
                }
                forumSettingsPage($errorMessage);
                exit;
            } else {
                $forumAddress = $response->{'forumAddress'};
                $forumApiKey  = $response->{'forumApiKey'};
                if (variable_get('forum_username') == '') {
                    variable_set('forum_username', $forumUsername);
                } elseif (variable_get('forum_username') != $forumUsername) {
                    variable_del('forum_username');
                    variable_set('forum_username', $forumUsername);
                }
                
                if (variable_get('forum_api_key') == '') {
                    variable_set('forum_api_key', $forumApiKey);
                } elseif (variable_get('forum_api_key') != $forumApiKey) {
                    variable_del('forum_api_key');
                    variable_set('forum_api_key', $forumApiKey);
                }
                
                if (variable_get('forum_url') == '') {
                    variable_set('forum_url', $forumAddress);
                } elseif (variable_get('forum_url') != $forumAddress) {
                    variable_del('forum_url');
                    variable_set('forum_url', $forumAddress);
                }
            }
        }
    }
    
    
    function getForumDetails($forumUsername, $forumPassword, $forumEmbeddedOption)
    {
        global $base_url;
        $data             = array(
            'action' => 'checkPluginLogin',
            'username' => $forumUsername,
            'password' => $forumPassword,
            'login_page_url' => $base_url . "/user",
            'logout_page_url' => $base_url . "/user/logout",
            'registration_url' => $base_url . "/user/register"
        );
        $options          = array(
            'type' => 'json',
            'method' => 'POST',
            'data' => drupal_http_build_query($data),
            'timeout' => 15,
            'headers' => array(
                'Content-Type' => 'application/json'
            )
        );
        $result           = drupal_http_request(WTB_SETTINGS_URL, $options);
        $response         = json_decode($result->data);
        $flag             = 0;
        $_SESSION['flag'] = 0;
        $forumUrl         = $response->{'forumAddress'};
        if ($forumUrl) {
            $forumApiKey   = $response->{'forumApiKey'};
            $forumUsername = $forumUsername;
            if ($forumEmbeddedOption == 1) {
                #add embed code and set it as a link if "embed" option is disabled.            
                createEmbeddedForum($response, $forumEmbeddedOption);
                global $base_url;
                $menu = menu_load_links('main-menu');
                for ($i = 0; $i < count($menu); $i++) {
                    if ($menu[$i]['link_title'] == 'Forum') {
                        $menu_link = $menu[$i]['link_path'];
                    }
                }
                updateForumData($forumUsername, $forumApiKey, $base_url . "/Forum");
            } else {
                #Remove embed code and set it as a link if "embed" option is disabled.            
                createForumMenuUrl($response);
                updateForumData($forumUsername, $forumApiKey);
            }
            return $result;
        } else {
            $_SESSION['flag'] = 1;
            $urlRedirect      = $_SERVER['HTTP_REFERER'];
            header("location:" . $urlRedirect);
            exit();
        }
    }
    
    function createEmbeddedForum($response, $forumEmbeddedOption)
    {
        $nodeResult = checkForumExist();
        $forumUrl   = $response->{'forumAddress'};
        if (empty($nodeResult)) {
            global $user;
            $summary = '';
            $node    = (object) array(
                'uid' => $user->uid,
                'name' => (isset($user->name) ? $user->name : ''),
                'type' => 'page',
                'language' => LANGUAGE_NONE,
                'title' => 'Forum'
            );
            node_save($node);
            $nodeId = $node->nid;
            $vid    = $node->vid;
        } else {
            $node    = node_load(array_shift(array_keys($nodeResult['node'])));
            $nodeId  = $node->nid;
            $vid     = $node->vid;
            $summary = $node->body['und'][0][summary];
        }
        if ($summary == '') {
            db_insert('field_data_body')->fields(array(
                'entity_type' => 'node',
                'bundle' => 'page',
                'deleted' => 0,
                'entity_id' => $nodeId,
                'delta' => 0,
                'revision_id' => $vid,
                'language' => 'und',
                'delta' => 0,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>',
                'body_summary' => $forumUrl,
                'body_format' => 'full_html'
            ))->execute();
            db_insert('field_revision_body')->fields(array(
                'entity_type' => 'node',
                'bundle' => 'page',
                'deleted' => 0,
                'entity_id' => $nodeId,
                'delta' => 0,
                'revision_id' => $vid,
                'language' => 'und',
                'delta' => 0,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>',
                'body_summary' => $forumUrl,
                'body_format' => 'full_html'
            ))->execute();
            
            $checkMenuExist = menu_get_item($forumUrl);
            $menu           = menu_load_links('main-menu');
            for ($i = 0; $i < count($menu); $i++) {
                if ($menu[$i]['link_title'] == 'Forum') {
                    menu_link_delete(NULL, $menu[$i]['link_path']);
                }
            }
            $menuUrl = 'node/' . $nodeId;
            $item    = array(
                'link_path' => $menuUrl,
                'link_title' => 'Forum',
                'menu_name' => 'main-menu', // Menu machine name, for example: main-menu
                'weight' => 0,
                'language' => $node->language,
                'plid' => 0, // Parent menu item, 0 if menu item is on top level
                'module' => 'menu'
            );
            menu_link_save($item);
            
            $path = array(
                'source' => "node/" . $nodeId,
                'alias' => 'Forum'
            );
            path_save($path);
            cache_clear_all('*', 'cache_menu', TRUE);
        } elseif ($summary != $forumUrl) {
            db_update('field_data_body')->fields(array(
                'body_summary' => $forumUrl,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>'
            ))->condition('entity_id', $nodeId, '=')->execute();
            db_update('field_revision_body')->fields(array(
                'body_summary' => $forumUrl,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>'
            ))->condition('entity_id', $nodeId, '=')->execute();
        }
        cache_clear_all('*', 'cache_menu', TRUE);
        
    }
    function updateForumData($forumUsername, $forumAPI, $embedUrl = "")
    {
        global $base_url;
        $data    = array(
            'action' => 'modifySSOURLs',
            'forumUsername' => $forumUsername,
            'forumApikey' => $forumAPI,
            'embed_page_url' => $embedUrl,
            'login_page_url' => $base_url . "/user",
            'logout_page_url' => $base_url . "/user/logout",
            'registration_url' => $base_url . "/user/register"
        );
        $options = array(
            'type' => 'json',
            'method' => 'POST',
            'data' => drupal_http_build_query($data),
            'timeout' => 15
            
        );
        $result  = drupal_http_request(WTB_SETTINGS_URL, $options);
        if (!$result) {
            drupal_set_message($result->error);
            die();
        }
    }
    /*create url if embedded is disabled*/
    function createForumMenuUrl($response)
    {
        $forumUrl = $response->{'forumAddress'};
        
        $checkMenuExist = menu_get_item($forumUrl);
        $nodeResult     = checkForumExist();
        $node           = node_load(array_shift(array_keys($nodeResult['node'])));
        node_delete($node->nid);
        if (!$checkMenuExist) {
            $menu = menu_load_links('main-menu');
            for ($i = 0; $i < count($menu); $i++) {
                if ($menu[$i]['link_title'] == 'Forum') {
                    menu_link_delete(NULL, $menu[$i]['link_path']);
                }
            }
            $item = array(
                'link_path' => $forumUrl,
                'link_title' => 'Forum',
                'menu_name' => 'main-menu', // Menu machine name, for example: main-menu
                'weight' => 0,
                'language' => $node->language,
                'plid' => 0, // Parent menu item, 0 if menu item is on top level
                'module' => 'menu'
            );
            menu_link_save($item);
            cache_clear_all('*', 'cache_menu', TRUE);
        }
    }
}
?>