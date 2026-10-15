<?php
/**
 * @file
 * Contains \Drupal\website_toolbox_forum\Form\WebsitetoolboxforumSync.
 */
namespace Drupal\website_toolbox_forum\Form;
const WT_SETTINGS_URL = 'https://www.websitetoolbox.com/tool/members/mb/settings';
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\node\Entity\Node;
use Drupal\menu_link_content\Entity\MenuLinkContent;
use Drupal\Core\Database\Database;
use Drupal\Core\Render\Element;
/**
 * Construct the storage changes in a configuration synchronization form.
 */
class WebsitetoolboxSync extends FormBase {
    public function getFormId() {
        return 'config_admin_import_form';
    }
    public function buildForm(array $form, FormStateInterface $form_state) {    

        $form                   = array();
        $html                   = '<h2>Forums by Website Toolbox</h2><br /><b>Please log in to your Website Toolbox account.</b><p>Not a Website Toolbox Forum owner? <a href="https://www.websitetoolbox.com/wordpress" target="_blank">Create a Forum Now!</a></p><p>Please <a href="https://www.websitetoolbox.com/contact?subject=WordPress+Plugin+Setup+Help" target="_blank">Contact Customer Support</a> if you need help getting set up.</p>';
        $form['display_html'][] = array(
            '#type' => 'markup',
            '#markup' => $html
        );
        $form['#attached']['html_head'][] = array(
            array(
                '#tag' => 'style',
                '#value' => '.description{ margin-left: 191px !important; }',
                '#attributes' => array(),
            ),
            'website_toolbox_forum:form_style',
        );
        if (isset($_SESSION['flag']) && ($_SESSION['flag'] == 1)) { 
            $getUserName            = \Drupal::config('node.settings')->get('forum_username', "");
            $html_error             = '<div class="messages messages--error">The username '.$getUserName.' does not exist. Try a different username or email address.</div>';
            $form['display_html'][] = array(
                '#type' => 'markup',
                '#markup' => $html_error
            );
        }
        if ((\Drupal::config('node.settings')->get('forum_username') == '') || (isset($_GET['forumSettingOption']) && $_GET['forumSettingOption'] == 1)) {
        $form['forum_username'] = array(
            '#type' => 'textfield',
            '#title' => t('Website Toolbox Username'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_username', ""),
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => TRUE
        );
        $form['forum_password'] = array(
            '#type' => 'password',
            '#title' => t('Website Toolbox Password'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_password', ""),
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => TRUE
        );
        $form['forum_hidden'] = array(
            '#type' => 'hidden',
            '#default_value' => \Drupal::config('node.settings')->get('forum_embedded_option', ""),
        );
        $form['user_sync_hidden'] = array(
            '#type' => 'hidden',
            '#default_value' => \Drupal::config('node.settings')->get('forum_roles_sync_option', ""),
        );
    } else {
        $html1                  = "<div><b>Website Toolbox Username &nbsp;  </b>" . \Drupal::config('node.settings')->get('forum_username') . "&nbsp;&nbsp;&nbsp;    
            <a href='?forumSettingOption=1' style='text-decoration:underline !important;'>Change</a></div>";
        $form['display_html'][] = array(
            '#type' => 'markup',
            '#markup' => $html1
        );        
    }
    $username = \Drupal::config('node.settings')->get('forum_username', "");
    if ((!isset($_GET['forumSettingOption'])) && ($username != '')) {
        $form['forum_embedded_label'] = array(
            '#type' => 'label',
            '#prefix' => '<div>',
            '#title' => t('<b>Embedded  </b>'),
            '#attributes'=>array('style'=>"margin-top:20px;margin-bottom:-25px;"),
        );
        $form['forum_embedded_option'] = array(
            '#type' => 'checkbox',
            '#title' => t('<b>Yes </b>'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_embedded_option'),
            '#size' => 60,
            '#description'=>'Enable this option to have your forum embedded in a page of your website. Disable this option to have your forum load in a full-sized window. You can use the Layout section in your Website Toolbox account to <a href="https://www.websitetoolbox.com/support/making-your-forum-layout-match-your-website-148" target="_blank">customize your forum layout to match your website</a> or <a href="https://www.websitetoolbox.com/contact?subject=Customize+Forum+Layout" target="_blank">contact Website Toolbox support to customize it for you</a>',
            '#required' => FALSE,
            '#attributes'=>array('style'=>'margin-top: -15px !important;margin-left: 191px !important;'),
            '#suffix' => '</div>'
        );
        $form['forum_roles_sync_option_label'] = array(
            '#type' => 'label',
            '#prefix' => '<div>',
            '#title' => t('<b>Synchronize Roles  </b>'),
            '#attributes'=>array('style'=>"margin-top:20px;margin-bottom:-25px;"),

        );

        $form['forum_roles_sync_option'] = array(
            '#type' => 'checkbox',
            '#title' => t('<b>Yes</b>'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_roles_sync_option'),
            '#suffix'=>'</div>',
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => FALSE,  
            '#attributes'=>array('style'=>"margin-top: -15px !important;margin-left: 191px !important;"),
        );         
        $form['forum_roles_sync_help'] = array(
            '#type' => 'label',
            '#title' =>'<img src ="../../../core/misc/icons/787878/questionmark-disc.svg" style="padding-top:12px;">',           
            '#default_value' => \Drupal::config('node.settings')->get('forum_roles_sync_option'),
            '#attributes' => array(
                'title' => 'If this option is enabled, user groups are created, updated and deleted in your Website Toolbox forum when the corresponding role in Drupal is created/edited/deleted. When a user is created/ edited in Drupal, user will be assigned the same user groups as their Drupal roles. There is no synchronization the other way around. When a user group is created in your forum, a role is not created on Drupal.',
                'style'=>'margin-left: 125px;margin-top:-35px;'
            ),
         );
    } 
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
      '#attributes' => array('style'=>'margin-top:25px;'),
    );
    return $form;       
  }
  public function submitForm(array &$form, FormStateInterface $form_state) {
    if(isset($_POST['forum_username'])){
        $forumUsername = $_POST['forum_username'];
        \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_username', $forumUsername)->save();
    }else{
        $forumUsername = \Drupal::config('node.settings')->get('forum_username');
    }
    if(isset($_POST['forum_password'])){
        $forumPassword = $_POST['forum_password'];
        \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_password', $forumPassword)->save();
    } else{
        $forumPassword = \Drupal::config('node.settings')->get('forum_password');
    }
    $username = \Drupal::config('node.settings')->get('forum_username', "");
    if(!isset($username)){
         $forumEmbeddedOption = 1;
         $userRolesSyncOption = 1;
    }else{
         if (isset($_POST['forum_embedded_option'])) {
            if ($_POST['forum_embedded_option'] == 1) {
                $forumEmbeddedOption = 1;
            } else {
                $forumEmbeddedOption = 0;
            }
        } else {
            if (isset($_POST['forum_hidden'])) {
                $forumEmbeddedOption = $_POST['forum_hidden'];
            }
        }  
        if (isset($_POST['forum_roles_sync_option'])) {
            if ($_POST['forum_roles_sync_option'] == 1) {
                $userRolesSyncOption = 1;
            } else {
                $userRolesSyncOption = 0;
            }
        } else {
            $userRolesSyncOption = $_POST['user_sync_hidden'];
        }  
    }   
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_embedded_option', $forumEmbeddedOption)->save();
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_roles_sync_option',$userRolesSyncOption)->save();
    checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption);  
    existingUserForumLogin();
    $urlRedirect      = explode("?",$_SERVER['HTTP_REFERER']);
    header("location:" . $urlRedirect[0]);
    exit();      
  }
}
function checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption){ 
    global $base_url;
    $request = \Drupal::httpClient()->post(WT_SETTINGS_URL, [
    'verify' => true,
        'form_params' => [
        'action' => 'checkPluginLogin',
        'plugin' => 'drupal',
        'username' => $forumUsername,
        'password' => $forumPassword,
        'login_page_url' => $base_url . "/?q=user",
        'logout_page_url' => $base_url . "/?q = user/logout",
        'registration_url' => $base_url . "/?q=user/register",]
        ])->getBody()->getContents(); 
    $response = json_decode($request);
    $forumUrl = $response->forumAddress; 
    $forumApiKey = $response->forumApiKey; 
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_api_key', $forumApiKey)->save();
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_url', $forumUrl)->save();
     if ($forumUrl) { 
        $database = \Drupal::database();
        $query      = \Drupal::entityQuery('node')->accessCheck(FALSE)->condition('title', 'Forum');
        $nids       = $query->execute();
        $getNodeArray = array_keys($nids);
        $getRevisionNodeID = array_shift($getNodeArray); 
        $existNode = $getRevisionNodeID ? Node::load($getRevisionNodeID) : NULL;
        $getNodeId = array_values($nids);
        $nodeId = 'internal:/node/'.($getNodeId[0] ?? '');
        if($existNode){
            $existNode->delete();   
        }  
        $rows       = $database->query("select * from {menu_link_content_data} where 
        (link__uri = '".$nodeId."' OR link__uri LIKE '%websitetoolbox.com' OR link__uri LIKE '%forumchitchat.com') ")->fetch();
        if($rows){
            $getNodeLinkId = $rows->id;  
            if($getNodeLinkId != ''){
                $menu_link = MenuLinkContent::load($getNodeLinkId);
                $menu_link->delete();
            }
        }
        unset($_SESSION['flag']);
        if($forumEmbeddedOption == 1){
            createEmbeddedForum($response, $forumEmbeddedOption,$nids);
            updateForumData($forumUsername, $forumApiKey, $base_url . "/Forum");
        }else{
            createForumMenuUrl($response,$nids);
            updateForumData($forumUsername, $forumApiKey);
        }
    }else{
        $_SESSION['flag'] = 1;
        $urlRedirect      = $_SERVER['HTTP_REFERER'];
        header("location:" . $urlRedirect);
        exit();
    }
}
function createEmbeddedForum($response, $forumEmbeddedOption,$nids){
    $forumUrl   = $response->forumAddress;  
    $node = Node::create(array('type' => 'page','title' => 'Forum','langcode' => 'en','uid' => '1','status' => 1));
    $node->body = array('format' => 'full_html', 'value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>');
    $node->save();
    $nodeId = $node->id(); 
    $menuLink = MenuLinkContent::create([
        'title' => 'Forum',
        'link' => ['uri' => 'internal:/node/' . $nodeId],
        'menu_name' => 'main',
        'expanded' => TRUE,]);
    $menuLink->save();
    saveForumPathAlias("/node/" . $nodeId, "/forum");
    drupal_static_reset();
    // drupal_flush_all_caches() removed: without the $kernel argument it rebuilds
    // the container mid-request, replacing the session service; SessionManager
    // then treats the in-flight session as obsolete and destroys/never-saves it,
    // logging the user out. Entity saves already invalidate the relevant caches
    // via cache tags (node, menu_link_content), so a full flush is unnecessary.
}
/*Update forum url and login/logout url on forum server configuration*/
function updateForumData($forumUsername, $forumAPI, $embedUrl = ""){
     global $base_url;    
     $request = \Drupal::httpClient()->post(WT_SETTINGS_URL, [
         'verify' => true,
         'form_params' => [
         'action' => 'modifySSOURLs',
         'plugin' => 'drupal',
         'forumUsername' => $forumUsername,
         'forumApikey' => $forumAPI,
         'embed_page_url' => $embedUrl,
         'login_page_url' => $base_url . "/user/register",
         'logout_page_url' => $base_url . "/user/logout",
         'registration_url' => $base_url . "/user/register",
    ]])->getBody()->getContents();    
}
    /*create url if embedded is disabled*/
function createForumMenuUrl($response,$nids){
    $forumUrl = $response->forumAddress;
    $menuLink = MenuLinkContent::create(['title' => 'Forum','link' => ['uri' => $forumUrl],'menu_name' => 'main','expanded' => TRUE,]);
    $menuLink->save();     
    saveForumPathAlias("/".$forumUrl, "/forum");
    drupal_static_reset();
    // drupal_flush_all_caches() removed: without the $kernel argument it rebuilds
    // the container mid-request, replacing the session service; SessionManager
    // then treats the in-flight session as obsolete and destroys/never-saves it,
    // logging the user out. Entity saves already invalidate the relevant caches
    // via cache tags (node, menu_link_content), so a full flush is unnecessary.
}
/*Save a path alias for a system path, replacing any existing alias for it.
 * Replaces the path.alias_storage service, which no longer exists in Drupal 10.*/
function saveForumPathAlias($path, $alias){
    $storage = \Drupal::entityTypeManager()->getStorage('path_alias');
    foreach ($storage->loadByProperties(['path' => $path]) as $existingAlias) {
        $existingAlias->delete();
    }
    $storage->create(['path' => $path, 'alias' => $alias, 'langcode' => 'und'])->save();
}
