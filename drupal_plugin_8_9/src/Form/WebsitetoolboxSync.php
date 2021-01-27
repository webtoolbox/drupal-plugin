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
            '#prefix' => '<div class="classLabel">',
            '#title' => t('<b>Embedded  </b>'),
            );
        $form['forum_embedded_option'] = array(
            '#type' => 'checkbox',
            '#title' => t('<b>Yes </b>'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_embedded_option'),
            '#size' => 60,
            '#description'=>'Enable this option to have your forum embedded in a page of your website. Disable this option to have your forum load in a full-sized window. You can use the Layout section in your Website Toolbox account to <a href="https://www.websitetoolbox.com/support/making-your-forum-layout-match-your-website-148" target="_blank">customize your forum layout to match your website</a> or <a href="https://www.websitetoolbox.com/contact?subject=Customize+Forum+Layout" target="_blank">contact Website Toolbox support to customize it for you</a>',
            '#required' => FALSE,
            '#suffix' => '</div>'
        );
        $form['forum_roles_sync_option_label'] = array(
            '#type' => 'label',
            '#prefix' => '<div class="classLabel">',
            '#title' => t('<b>Synchronize Roles  </b>'),
            );
        $form['forum_roles_sync_help'] = array(
            '#type' => 'label',
            '#title' =>'<img src ="../../../core/misc/icons/787878/questionmark-disc.svg">',           
            '#default_value' => \Drupal::config('node.settings')->get('forum_roles_sync_option'),
            '#attributes' => array(
                'title' => 'If this option is enabled, user groups are created, updated and deleted in your Website Toolbox forum when the corresponding role in Drupal is created/edited/deleted.
                When a user is created/ edited in Drupal, user will be assigned the same user groups as their Drupal roles.
                There is no synchronization the other way around. When a user group is created in your forum, a role is not created on Drupal.',
                'class'=>'iconHelp')
        );
        $form['forum_roles_sync_option'] = array(
            '#type' => 'checkbox',
            '#title' => t('<b>Yes</b>'),
            '#default_value' => \Drupal::config('node.settings')->get('forum_roles_sync_option'),
            '#suffix'=>'</div>',
            '#size' => 60,
            '#maxlength' => 250,
            '#required' => FALSE,  
        );         
     } 
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
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
    if(isset($username)){
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
            $forumEmbeddedOption = $_POST['forum_hidden'];
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
        'username' => $forumUsername,
        'password' => $forumPassword,
        'login_page_url' => $base_url . "/?q=user",
        'logout_page_url' => $base_url . "/?q = user/logout",
        'registration_url' => $base_url . "/?q=user/register",],
        'headers' => ['Content-type' => 'application/json',],])->getBody()->getContents(); 
    $response = json_decode($request);
    $forumUrl = $response->forumAddress; 
    $forumApiKey = $response->forumApiKey; 
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_api_key', $forumApiKey)->save();
    \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_url', $forumUrl)->save();
     if ($forumUrl) { 
        $database = \Drupal::database();
        $query      = \Drupal::entityQuery('node')->condition('title', 'Forum');
        $nids       = $query->execute();
        $getNodeArray = array_keys($nids);
        $getNodeID = array_shift($getNodeArray);
        $existNode = Node::load($getNodeID);
        if($existNode){
            $existNode->delete();   
        } 
        $rows       = $database->query("select * from {menu_link_content_data} where title = 'Forum'")->fetch();
        $getNodeLinkId = $rows->id;  
        if($getNodeLinkId != ''){
            $menu_link = MenuLinkContent::load($getNodeLinkId);
            $menu_link->delete();
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
    $node = Node::create(array(
        'type' => 'page',
        'title' => 'Forum',
        'langcode' => 'en',
        'uid' => '1',
        'status' => 1
        )
    );
    $node->body = array(
        'format' => 'full_html',
        'value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>'
    );
    $node->save();
    $nodeId = $node->id(); 
    $menuLink = MenuLinkContent::create([
        'title' => 'Forum',
        'link' => ['uri' => 'internal:/node/' . $nodeId],
        'menu_name' => 'main',
        'expanded' => TRUE,]);
    $menuLink->save();
    $path_alias_storage = \Drupal::entityTypeManager()->getStorage('path_alias');      
    $path_alias = $path_alias_storage->create([
        'path' => "/node/" . $nodeId,
        'alias' => "/forum",
        'langcode' => "und",
    ]);
    $path_alias->save();
    drupal_static_reset();
    drupal_flush_all_caches();  
}
/*Update forum url and login/logout url on forum server configuration*/
function updateForumData($forumUsername, $forumAPI, $embedUrl = ""){
     global $base_url;    
     $request = \Drupal::httpClient()->post(WT_SETTINGS_URL, [
         'verify' => true,
         'form_params' => [
         'action' => 'modifySSOURLs',
         'forumUsername' => $forumUsername,
         'forumApikey' => $forumAPI,
         'embed_page_url' => $embedUrl,
         'login_page_url' => $base_url . "/user/register",
         'logout_page_url' => $base_url . "/user/logout",
         'registration_url' => $base_url . "/user/register",]])->getBody()->getContents();    
}
    /*create url if embedded is disabled*/
function createForumMenuUrl($response,$nids){
    $forumUrl = $response->forumAddress;
    $menuLink = MenuLinkContent::create(['title' => 'Forum','link' => ['uri' => $forumUrl],'menu_name' => 'main','expanded' => TRUE,]);
    $menuLink->save();     
    $path_alias_storage = \Drupal::entityTypeManager()->getStorage('path_alias');      
    $path_alias = $path_alias_storage->create([
        'path' => $forumUrl,
        'alias' => "/forum",
        'langcode' => "und", 
    ]);
    drupal_static_reset();
    drupal_flush_all_caches();      
}
