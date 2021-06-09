<?php  
/*create module admin configuration form*/

function websitetoolbox_admin_form(){
    $form                   = array();
    $html                   = '<h2>Forums by Website Toolbox</h2><br /><b>Please log in to your Website Toolbox account.</b><p>Not a Website Toolbox Forum owner? <a href="https://www.websitetoolbox.com/forum/?drupal" target="_blank">Create a Forum Now!</a></p><p>Please <a href="https://www.websitetoolbox.com/contact?subject=Drupal+Plugin+Setup+Help" target="_blank">Contact Customer Support</a> if you need help getting set up.</p>';    
    if ((variable_get('websitetoolbox_forum_username') == '') || (isset($_GET['forumSettingOption']) && $_GET['forumSettingOption'] == 1)) {
        $form['websitetoolbox_forum_username'] = array(
            '#type'         => 'textfield',
            '#title'        => t('Website Toolbox Username'),
            '#default_value'=> variable_get('websitetoolbox_forum_username', ""),
            '#size'         => 60,
            '#maxlength'    => 250,
            '#required'     => TRUE
        );
        $form['websitetoolbox_forum_password'] = array(
            '#type'         => 'password',
            '#title'        => t('Website Toolbox Password'),
            '#default_value'=> variable_get('websitetoolbox_forum_password', ""),
            '#size'         => 60,
            '#maxlength'    => 250,
            '#required'     => TRUE
        );
        $form['embeddedValue'] = array(
            '#type'     => 'hidden',
            '#title'    => 'embeddedValue'
         );        
    } else {
        $html1                  = "<div style='margin-bottom:20px;'><b>Website Toolbox Username  &nbsp;  </b>" . variable_get('websitetoolbox_forum_username') . "&nbsp;&nbsp;&nbsp;    
            <a href='?q=" . $_GET['q'] . "&forumSettingOption=1'>Change</a></div>";
        $form['display_html'][] = array(
            '#type'     => 'markup',
            '#markup'   => $html1
        );        
    }
    if (!isset($_GET['forumSettingOption'])) {
        $getUsername = variable_get('websitetoolbox_forum_username');
        if(!$getUsername){
            $embeddedOption = 1;
        }else{
            $embeddedOption = variable_get('websitetoolbox_embedded_option');
            $labelEmbedded                   = "<b>Embedded</b>";
            $form['websitetoolbox_embedded_option']   = array(
                '#type'         => 'checkbox',
                '#title'        => t('<b>Yes </b>'),
                '#default_value'=> $embeddedOption,
                '#size'         => 60,
                '#maxlength'    => 250,
                '#description'  => 'Enable this option to have your forum embedded in a page of your website. Disable this option to have your forum load in a full-sized window. You can use the Layout section in your Website Toolbox account to <a href="https://www.websitetoolbox.com/support/making-your-forum-layout-match-your-website-148" target="_blank">customize your forum layout to match your website</a> or <a href="https://www.websitetoolbox.com/contact?subject=Customize+Forum+Layout" target="_blank">contact Website Toolbox support to customize it for you</a>',
                '#required'     => FALSE,
                '#prefix'       => '<label for="edit-firstname">' . t('<b>Embedded</b>'),
                '#suffix'       => '</label>'
            );
        }
        $form['websitetoolbox_roles_sync_option'] = array(
            '#type'             => 'checkbox',
            '#title'            => t('<b>Yes</b>'),
            '#default_value'    => variable_get('websitetoolbox_roles_sync_option'),
            '#prefix'           => '<label for="edit-firstname">' . t('<b>Synchronize Roles</b>'),
            '#suffix'           => '</label>',
            '#size'             => 60,
            '#maxlength'        => 250,
            '#required'         => FALSE,
            '#description'      => 'If this option is enabled, user groups are created, updated and deleted in your Website Toolbox forum when the corresponding role in Drupal is created/edited/deleted. When a user is created/ edited in Drupal, user will be assigned the same user groups as their Drupal roles. There is no synchronization the other way around. When a user group is created in your forum, a role is not created on Drupal.'
        );
    }
    $form['actions']            = array(
        '#type' => 'actions'
    );
    $form['actions']['submit']  = array(
        '#type' => 'submit',
        '#value' => t('Save Configuration')
    );
    $form['#attached']['css'][] = drupal_get_path('module', 'websitetoolbox') . '/websitetoolbox.css';
    $form['#submit'][]          = 'websitetoolbox_admin_form_submit';
    return $form;
}
/*create module admin configuration form*/
/*submit username and password for forum configuation */
function websitetoolbox_admin_form_submit($form,&$formState){
    if (isset($formState['values']['websitetoolbox_forum_username']) && $formState['values']['websitetoolbox_forum_username'] != '') {
        $forumUsername = $formState['values']['websitetoolbox_forum_username'];
    } else {
        $forumUsername = variable_get('websitetoolbox_forum_username');
    }
    if (isset($formState['values']['websitetoolbox_forum_password']) && $formState['values']['websitetoolbox_forum_password'] != '') {
        $forumPassword = $formState['values']['websitetoolbox_forum_password'];
    } else {
        $forumPassword = variable_get('websitetoolbox_forum_password');
    }
    if (isset($formState['values']['websitetoolbox_embedded_option'])) { 
        $forumEmbeddedOption = $formState['values']['websitetoolbox_embedded_option'];
    } else { 
        $forumEmbeddedOption = variable_get('websitetoolbox_embedded_option');
    }
    if (isset($formState['values']['websitetoolbox_roles_sync_option'])) { 
        $forumUserRoleSync = $formState['values']['websitetoolbox_roles_sync_option'];
    } else { 
        $forumUserRoleSync = variable_get('websitetoolbox_roles_sync_option');
    }    
    
    websitetoolbox_check_forum_listed($forumUsername, $forumPassword, $forumEmbeddedOption,$forumUserRoleSync);
    websitetoolbox_existing_user_login();
    drupal_flush_all_caches();
}

function websitetoolbox_check_forum_listed($forumUsername, $forumPassword, $forumEmbeddedOption,$forumUserRoleSync){
    $result = websitetoolbox_get_forum_details($forumUsername, $forumPassword, $forumEmbeddedOption);
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
            variable_set('websitetoolbox_forum_username', $forumUsername);
            variable_set('websitetoolbox_forum_password', $forumPassword);
            variable_set('websitetoolbox_forum_api_key', $forumApiKey);
            variable_set('websitetoolbox_forum_url', $forumAddress); 
            variable_set('websitetoolbox_roles_sync_option', $forumUserRoleSync);         
        }
    }
}
/*get forum username, password, API key */
function websitetoolbox_get_forum_details($forumUsername, $forumPassword, $forumEmbeddedOption)
{
    $loginPageUrl       = url('user/login',array('absolute' => TRUE));
    $logoutPageUrl      = url('user/logout',array('absolute' => TRUE));
    $registrationUrl    = url('user/register',array('absolute' => TRUE));
    $data               = array(
        'action'            => 'checkPluginLogin',
        'username'          => $forumUsername,
        'password'          => $forumPassword,
        'login_page_url'    => $loginPageUrl,
        'logout_page_url'   => $logoutPageUrl,
        'registration_url'  => $registrationUrl,
        'plugin'            => 'drupal'
    );
    $data               = drupal_http_build_query($data);
    $result             = websitetoolbox_send_api_request('POST', WT_SETTINGS_URL, $data);        
    $response           = json_decode($result->data);
    $flag               = 0;
    $_SESSION['flag']   = 0;
    $forumUrl           = $response->{'forumAddress'};
    if ($forumUrl) {
        $forumApiKey    = $response->{'forumApiKey'};   
        $getUsername    = variable_get('websitetoolbox_forum_username');
        if(!$getUsername){
            variable_set('websitetoolbox_embedded_option', 1);
        }     else{
            variable_set('websitetoolbox_embedded_option', $forumEmbeddedOption);
        }       
        if ($forumEmbeddedOption == 1 || (!$getUsername) ) { 
            #add embed code and set it as a link if "embed" option is disabled.            
            $nodeId         = websitetoolbox_create_embedded_forum($response, $forumEmbeddedOption);
            $embeddedPath   = url("node/".$nodeId, array('absolute'=>TRUE)); 
            websitetoolbox_update_forum_data($forumUsername, $forumApiKey, $embeddedPath);
        } else {
            #Remove embed code and set it as a link if "embed" option is disabled.            
            websitetoolbox_create_forum_menu_url($response);
            websitetoolbox_update_forum_data($forumUsername, $forumApiKey);
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
function websitetoolbox_create_embedded_forum($response, $forumEmbeddedOption){ 
    $nid        = variable_get("websitetoolbox_nodeId");
    $forumUrl   = $response->{'forumAddress'};
    if($nid){
        $node   = node_load($nid);
        if($node->body){  
            $bodyContent    = $node->body['und'][0]['value'];            
            if($bodyContent && (strpos($bodyContent, $forumUrl) === false)) {
                 $node->body['und'][0]['value']  ='<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Website Toolbox Forum</a></noscript></div>';
                 $node->format  = 'full_html';
                 node_save($node); 
            }
        }else{ 
             $node->body['und'][0]['value']  ='<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Website Toolbox Forum</a></noscript></div>';
             $node->format  = 'full_html';
             node_save($node);
        }
        $node->status   = 1;
        node_save($node); 
        websitetoolbox_check_menu_exist("node/".$nid);
        cache_clear_all('*', 'cache_menu', TRUE);
    }else{
        $node               = new stdClass();
        $node->type         = 'page';
        $node->status       = 1;
        $node->uid          = 1;
        $node->title        = "forum";
        $node->created      = time();
        $node->timestamp    = time();
        $node->sticky       = 0;
        $node->format       = 'full_html';
        $node->language     = 'en';
        $node->body['und'][0]['value']  = '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Website Toolbox Forum</a></noscript></div>';
        $node->revision     = 0;
        node_save($node); 
        $nodeId             = $node->nid;
        variable_set("websitetoolbox_nodeId",$nodeId);
        $menuUrl            = 'node/' . $nodeId;
        $item               = array(
            'link_path'     => $menuUrl,
            'link_title'    => 'forum',
            'menu_name'     => 'main-menu', // Menu machine name, for example: main-menu
            'weight'        => 0,
            'language'      => $node->language,
            'plid'          => 0, // Parent menu item, 0 if menu item is on top level
            'module'        => 'menu'
        );
        $menuLinkId = menu_link_save($item);      
        variable_set("websitetoolbox_menuLinkId",$menuLinkId);
        $path = array(
            'source'    => "node/" . $nodeId,
            'alias'     => 'forum'
        );
        path_save($path);
        cache_clear_all('*', 'cache_menu', TRUE);
        return $nodeId;
    }

}

/*Update forum url and login/logout url on forum server configuration*/
function websitetoolbox_update_forum_data($forumUsername, $forumAPI, $embedUrl = ""){ 
    global $base_url;
    $loginPageUrl       = url('user/login',array('absolute' => TRUE));
    $logoutPageUrl      = url('user/logout',array('absolute' => TRUE));
    $registrationUrl    = url('user/register',array('absolute' => TRUE));
    $data               = array(
        'action'            => 'modifySSOURLs',
        'forumUsername'     => $forumUsername,
        'forumApikey'       => $forumAPI,
        'embed_page_url'    => $embedUrl,
        'login_page_url'    => $loginPageUrl,
        'logout_page_url'   => $logoutPageUrl,
        'registration_url'  => $registrationUrl,
        'plugin'            => 'drupal'
    ); 
    $data       = drupal_http_build_query($data);
    $result     = websitetoolbox_send_api_request('POST', WT_SETTINGS_URL, $data);
    if (!$result) {
        drupal_set_message($result->error);
    }
}
/*create url if embedded is disabled*/
function websitetoolbox_create_forum_menu_url($response){
    $forumUrl       = variable_get("websitetoolbox_forum_url");
    $menuLinkId     = variable_get("websitetoolbox_menuLinkId"); 
    $nid            = variable_get("websitetoolbox_nodeId");
    $node           = node_load($nid);
    $node->status   = 0;
    node_save($node); 
    websitetoolbox_check_menu_exist($forumUrl);
}
function websitetoolbox_check_menu_exist($menuUrl){
    $menuLinkId = variable_get("websitetoolbox_menuLinkId"); 
    $result = db_query("SELECT * FROM {menu_links} WHERE mlid = ".$menuLinkId)->fetchAll(PDO::FETCH_ASSOC);
    foreach ($result as $link) {
        $link['link_path']  = $menuUrl;
        $link['options']    = unserialize($link['options']);
        menu_link_save($link);
    }
}
