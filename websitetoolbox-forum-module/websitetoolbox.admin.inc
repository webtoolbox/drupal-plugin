<?php
/*create module admin configuration form*/
function adminConfigPageLayout()
{
    $form                   = array();
    $html                   = '<h2>Forums by Website Toolbox</h2><br /><b>Please log in to your Website Toolbox account.</b><p>Not a Website Toolbox Forum owner? <a href="https://www.websitetoolbox.com/wordpress" target="_blank">Create a Forum Now!</a></p><p>Please <a href="https://www.websitetoolbox.com/contact?subject=WordPress+Plugin+Setup+Help" target="_blank">Contact Customer Support</a> if you need help getting set up.</p>';
    $form['display_html'][] = array(
        '#type' => 'markup',
        '#markup' => $html
    );
    if ((variable_get('wtb_forum_username') == '') || $_GET['wtbSettingOption'] == 1) {
        $form['wtb_forum_username'] = array(
            '#type' => 'textfield',
            '#title' => t('Website ToolBox Username'),
            '#default_value' => variable_get('wtb_forum_username', ""),
            '#size' => 60,
            '#maxlength' => 250,
            '#description' => t("Website ToolBox Username"),
            '#required' => TRUE
        );
        $form['wtb_forum_password'] = array(
            '#type' => 'password',
            '#title' => t('Website ToolBox Password'),
            '#default_value' => variable_get('wtb_forum_password', ""),
            '#size' => 60,
            '#maxlength' => 250,
            '#description' => t("Website ToolBox Password"),
            '#required' => TRUE
        );
    } else {
        $html1                  = "<div><b>Website ToolBox Username :  </b>" . variable_get('wtb_forum_username') . "&nbsp;&nbsp;&nbsp;     <a href='?q=" . $_GET[q] . "&wtbSettingOption=1'>Change</a></div>";
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
    if ($_GET['wtbSettingOption'] != 1) {
        $form['wtb_forum_embedded'] = array(
            '#type' => 'checkbox',
            '#title' => t('Embedded'),
            '#default_value' => variable_get('wtb_forum_embedded', 1),
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => FALSE
        );
    }
    $form['#submit'][] = 'adminConfigPageSubmit';
    return system_settings_form($form);
}
/*create module admin configuration form*/
/*submit username and password for forum configuation */
function adminConfigPageSubmit($formId, &$formState)
{
    if ($_POST['wtb_forum_username'] != '') {
        $wtbForumUsername = $_POST['wtb_forum_username'];
    } else {
        $wtbForumUsername = variable_get('wtb_forum_username');
    }
    if ($_POST['wtb_forum_password'] != '') {
        $wtbForumPassword = $_POST['wtb_forum_password'];
    } else {
        $wtbForumPassword = variable_get('wtb_forum_password');
    }

    $wtbForumEmbedded = $_POST['wtb_forum_embedded'];
    if ($wtbForumEmbedded == 1) {
        $wtbForumEmbedded = 1;
    } else {
        $wtbForumEmbedded = 0;
    }
    wtbForumPluginCheck($wtbForumUsername, $wtbForumPassword, $wtbForumEmbedded);
}
/*submit username and password for forum configuation */
/*get authtoken, save authtoken, create cookies after user logged in*/
function loginToForum($wtbForumUsername, $wtbForumPassword, $wtbForumEmbedded)
{
    $data    = array(
        'action' => 'checkPluginLogin',
        'username' => $wtbForumUsername,
        'password' => $wtbForumPassword
    );
    $options = array(
        'type' => 'json',
        'method' => 'POST',
        'data' => drupal_http_build_query($data),
        'timeout' => 15,
        'headers' => array(
            'Content-Type' => 'application/json'
        )
    );
    $result  = drupal_http_request(WTB_SETTINGS_URL, $options);
    if ($result) {
        $response      = json_decode($result->data);
        $wtbForumUrl   = $response->{'forumAddress'};
        $forumAddress  = $response->{'forumAddress'};
        $forumApiKey  = $response->{'forumApiKey'};
        $forumUsername = $wtbForumUsername;
        if ($wtbForumEmbedded == 1) {
            #add embed code and set it as a link if "embed" option is disabled.
            updateForumData($forumUsername, $forumApiKey, $base_url . "/Forum");
            createEmbeddedForum($response, $wtbForumEmbedded);
        } else {
            #Remove embed code and set it as a link if "embed" option is disabled.
            updateForumData($forumUsername, $forumApiKey);
            createForumMenuUrl($response);

        }
        return $result;
    } else {
        drupal_set_message($result->error);
        die();
    }
}
/*update forum data according to embedded enabled or disabled*/
function updateForumData($forumUsername, $forumAPI, $embedUrl = "")
{
    $data    = array(
        'action' => 'modifySSOURLs',
        'forumUsername' => $forumUsername,
        'forumApikey' => $forumAPI,
        'embed_page_url' => $embedUrl
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
    $wtbForumUrl = $response->{'forumAddress'};
    updateForumData($forumUsername, $forumApiKey);
    $checkMenuExist = menu_get_item($wtbForumUrl);
    $nodeResult     = chkForumExist();
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
            'link_path' => $wtbForumUrl,
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
/*check if forumdata already exist*/
function chkForumExist(){
    $nodeResult = (new EntityFieldQuery())->entityCondition('entity_type', 'node')->propertyCondition('title', 'Forum')->execute();
    return $nodeResult;
}
function createEmbeddedForum($response, $wtbForumEmbedded)
{
    $nodeResult  = chkForumExist();
    $wtbForumUrl = $response->{'forumAddress'};
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
            'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $wtbForumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $wtbForumUrl . '">Forum</a></noscript></div>',
            'body_summary' => $wtbForumUrl,
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
            'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $wtbForumUrl . '"></script><noscript><a href="' . $wtbForumUrl . '">Forum</a></noscript></div>',
            'body_summary' => $wtbForumUrl,
            'body_format' => 'full_html'
        ))->execute();
        $checkMenuExist = menu_get_item($wtbForumUrl);
        if (!$checkMenuExist) {
            $menu = menu_load_links('main-menu');
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
            db_insert('url_alias')->fields(array(
                'source' => 'node/' . $nodeId,
                'alias' => 'Forum',
                'language' => 'und'
            ))->execute();
            cache_clear_all('*', 'cache_menu', TRUE);
        }

    } elseif ($summary != $wtbForumUrl) {
            db_update('field_data_body')->fields(array(
                'body_summary' => $wtbForumUrl,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $wtbForumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $wtbForumUrl . '">Forum</a></noscript></div>'
            ))->condition('entity_id', $nodeId, '=')->execute();
            db_update('field_revision_body')->fields(array(
                'body_summary' => $wtbForumUrl,
                'body_value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $wtbForumUrl . '"></script><noscript><a href="' . $wtbForumUrl . '">Forum</a></noscript></div>'
            ))->condition('entity_id', $nodeId, '=')->execute();
        }

}
/* Purpose: Execute a HTTP request to validate forum login provided by the user.
Parameter: forum username, forum password.
Return: JSON responce */
function wtbForumPluginCheck($wtbForumUsername, $wtbForumPassword, $wtbForumEmbedded)
{
    $result = loginToForum($wtbForumUsername, $wtbForumPassword, $wtbForumEmbedded);
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
            wtbForumSettingsPage($errorMessage);
            exit;
        } else {
            $forumAddress = $response->{'forumAddress'};
            $forumApiKey = $response->{'forumApiKey'};
            if (variable_get('wtb_username') == '') {
                variable_set('wtb_username', $wtbForumUsername);
            } elseif (variable_get('wtb_username') != $wtbForumUsername) {
                variable_del('wtb_username');
                variable_set('wtb_username', $wtbForumUsername);
            }

            if (variable_get('wtb_api_key') == '') {
                variable_set('wtb_api_key', $forumApiKey);
            } elseif (variable_get('wtb_api_key') != $forumApiKey) {
                variable_del('wtb_api_key');
                variable_set('wtb_api_key', $forumApiKey);
            }

            if (variable_get('wtb_forums_url') == '') {
                variable_set('wtb_forums_url', $forumAddress);
            } elseif (variable_get('wtb_forums_url') != $forumAddress) {
                variable_del('wtb_forums_url');
                variable_set('wtb_forums_url', $forumAddress);
            }
        }
    }
}
?>