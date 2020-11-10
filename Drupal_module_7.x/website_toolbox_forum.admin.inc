<?php
const WT_SETTINGS_URL = 'https://beta.websitetoolbox.com/tool/members/mb/settings';
/*create module admin configuration form*/
function website_toolbox_forum_config_page_layout(){
    $form                   = array();
    $html                   = '<h2>Forums by Website Toolbox</h2><br /><b>Please log in to your Website Toolbox account.</b><p>Not a Website Toolbox Forum owner? <a href="https://www.websitetoolbox.com/forum/?drupal" target="_blank">Create a Forum Now!</a></p><p>Please <a href="https://www.websitetoolbox.com/contact?subject=Drupal+Plugin+Setup+Help" target="_blank">Contact Customer Support</a> if you need help getting set up.</p>';
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
    if ((variable_get('forum_username') == '') || (isset($_GET['forumSettingOption']) && $_GET['forumSettingOption'] == 1)) {
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
        $html1                  = "<div><b>Website Toolbox Username :  </b>" . variable_get('forum_username') . "&nbsp;&nbsp;&nbsp;    
            <a href='?q=" . $_GET['q'] . "&forumSettingOption=1'>Change</a></div>";
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
    if (!isset($_GET['forumSettingOption'])) {
        $form['forum_embedded_option'] = array(
            '#type' => 'checkbox',
            '#title' => t('Embedded'),
            '#default_value' => variable_get('forum_embedded_option'),
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => FALSE
        );
    }
    $form['#submit'][] = 'website_toolbox_forum_config_page_submit';
    return system_settings_form($form);
}
/*create module admin configuration form*/
/*submit username and password for forum configuation */
function website_toolbox_forum_config_page_submit($formId, &$formState){
    if (isset($_POST['forum_username']) && $_POST['forum_username'] != '') {
        $forumUsername = check_plain($_POST['forum_username']);
    } else {
        $forumUsername = variable_get('forum_username');
    }
    if (isset($_POST['forum_password']) && $_POST['forum_password'] != '') {
        $forumPassword = check_plain($_POST['forum_password']);
    } else {
        $forumPassword = variable_get('forum_password');
    }
    if (isset($_POST['forum_embedded_option'])) {
        $forumEmbeddedOption = check_plain($_POST['forum_embedded_option']);
    }
    if (isset($forumEmbeddedOption)) {
        if ($forumEmbeddedOption == 1) {
            $forumEmbeddedOption = 1;
        } else {
            $forumEmbeddedOption = 0;
        }
    } else {
        if (isset($_POST['embeddedValue'])) {
            $forumEmbeddedOption = check_plain($_POST['embeddedValue']);
        } else {
            $forumEmbeddedOption = 0;
        }
    }
    website_toolbox_forum_check_forum_listed($forumUsername, $forumPassword, $forumEmbeddedOption);
    website_toolbox_forum_existing_user_forum_login();
    drupal_static_reset();
    drupal_flush_all_caches();
}
function website_toolbox_forum_check_forum_listed($forumUsername, $forumPassword, $forumEmbeddedOption){
    $result = website_toolbox_forum_get_forum_details($forumUsername, $forumPassword, $forumEmbeddedOption);
    if ($result) {
        $response = json_decode($result->data);
        if ($result->code != 200 || $response->{'errorMessage'}) {
            if ($result->code == 403) {
                $ip           = file_get_contents('https://api.ipify.org');
                $errorMessage = "Your IP address ($ip) is blocked. Please <a href='https://www.websitetoolbox.com/contact?subject=drupal+IP($ip)+Blocked' target='_blank'>Contact Website Toolbox</a>.";
            } elseif ($result->code != 200) {
                $errorMessage = $response->{'errorMessage'};
            } else {
                #show error message to the user.
                $errorMessage = $response->{'errorMessage'};
                $errorMessage = preg_replace('#^Error:#', '', $errorMessage);
            }
            drupal_set_message(t($errorMessage), 'error');
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
/*get forum username, password, API key */
function website_toolbox_forum_get_forum_details($forumUsername, $forumPassword, $forumEmbeddedOption){
    global $base_url;
    $data             = array(
        'action' => 'checkPluginLogin',
        'username' => $forumUsername,
        'password' => $forumPassword,
        'login_page_url' => $base_url . "/?q=user",
        'logout_page_url' => $base_url . "/?q = user/logout",
        'registration_url' => $base_url . "/?q=user/register"
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
    $result           = drupal_http_request(WT_SETTINGS_URL, $options);
    $response         = json_decode($result->data);
    $flag             = 0;
    $_SESSION['flag'] = 0;
    $forumUrl         = $response->{'forumAddress'};
    if ($forumUrl) {
        $forumApiKey   = $response->{'forumApiKey'};
        $forumUsername = $forumUsername;
        if ($forumEmbeddedOption == 1) {
            #add embed code and set it as a link if "embed" option is disabled.            
            website_toolbox_forum_create_embeddedforum($response, $forumEmbeddedOption);
            global $base_url;
            $menu = menu_load_links('main-menu');
            for ($i = 0; $i < count($menu); $i++) {
                if ($menu[$i]['link_title'] == 'Forum') {
                    $menu_link = $menu[$i]['link_path'];
                }
            }
            website_toolbox_forum_update_forum_data($forumUsername, $forumApiKey, $base_url . "/?q=" . $menu_link);
        } else {
            #Remove embed code and set it as a link if "embed" option is disabled.            
            website_toolbox_forum_create_forum_menu_url($response);
            website_toolbox_forum_update_forum_data($forumUsername, $forumApiKey);
        }
        return $result;
    } else {
        $_SESSION['flag'] = 1;
        $urlRedirect      = $_SERVER['HTTP_REFERER'];
        header("location:" . $urlRedirect);
        exit();
    }
}
/*If forum embedded if checked, save forum details in database*/
function website_toolbox_forum_create_embeddedforum($response, $forumEmbeddedOption){
    $nodeResult = website_toolbox_forum_check_forum_exist();
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
        $nodeKey   = array_keys($nodeResult['node']);
        $getNodeID = array_shift($nodeKey);
        $node      = node_load($getNodeID);
        $nodeId    = $node->nid;
        $vid       = $node->vid;
        $summary   = $node->body['und'][0]['summary'];
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
/*Update forum url and login/logout url on forum server configuration*/
function website_toolbox_forum_update_forum_data($forumUsername, $forumAPI, $embedUrl = ""){
    global $base_url;
    $data    = array(
        'action' => 'modifySSOURLs',
        'forumUsername' => $forumUsername,
        'forumApikey' => $forumAPI,
        'embed_page_url' => $embedUrl,
        'login_page_url' => $base_url . "/?q=user/register",
        'logout_page_url' => $base_url . "/?q=user/logout",
        'registration_url' => $base_url . "/?q=user/register"
    );
    $options = array(
        'type' => 'json',
        'method' => 'POST',
        'data' => drupal_http_build_query($data),
        'timeout' => 15
        
    );
    $result  = drupal_http_request(WT_SETTINGS_URL, $options);
    if (!$result) {
        drupal_set_message($result->error);
        die();
    }
}
/*create url if embedded is disabled*/
function website_toolbox_forum_create_forum_menu_url($response){
    $forumUrl = $response->{'forumAddress'};
    $checkMenuExist = menu_get_item($forumUrl);
    $nodeResult     = website_toolbox_forum_check_forum_exist();
    if (!empty($nodeResult)) {
        $node = node_load(array_shift(array_keys($nodeResult['node'])));
        node_delete($node->nid);
    }
    if (!empty($checkMenuExist)) {
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
            'language' => 'und',
            'plid' => 0, // Parent menu item, 0 if menu item is on top level
            'module' => 'menu'
        );
        menu_link_save($item);
        cache_clear_all('*', 'cache_menu', TRUE);
    }
}