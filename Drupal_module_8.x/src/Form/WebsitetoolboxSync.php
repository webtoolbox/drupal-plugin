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
      $htmlError             = '<div class="messages error">Please provide correct username and password</div>';
      $form['display_html'][] = array(
        '#type' => 'markup',
        '#markup' => $htmlError
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
    } else {
      $html1                  = "<div><b>Website Toolbox Username :  </b>" . \Drupal::config('node.settings')->get('forum_username') . "&nbsp;&nbsp;&nbsp;
      <a href='?forumSettingOption=1'>Change</a></div>";
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
          '#default_value' => \Drupal::config('node.settings')->get('forum_embedded_option'),
          '#size' => 60,
          '#maxlength' => 250,
          '#required' => FALSE
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
      if (isset($_POST['forum_embedded_option'])) {
        if ($_POST['forum_embedded_option'] == 1) {
          $forumEmbeddedOption = 1;
        } else {
          $forumEmbeddedOption = 0;
        }
      } else {
        $forumEmbeddedOption = $_POST['forum_hidden'];
      }
      \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_embedded_option', $forumEmbeddedOption)->save();
      checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption);
      $urlRedirect      = explode("?",$_SERVER['HTTP_REFERER']);
      header("location:" . $urlRedirect[0]);
      exit();
    }

  }
function checkForumListed($forumUsername, $forumPassword, $forumEmbeddedOption){
global $base_url;
$request = \Drupal::httpClient()->post(WT_SETTINGS_URL, [
  'verify' => true,
  'form_params' => ['action' => 'checkPluginLogin',
  'username' => $forumUsername,
  'password' => $forumPassword,
  'login_page_url' => $base_url . "/?q=user",
  'logout_page_url' => $base_url . "/?q = user/logout",
  'registration_url' => $base_url . "/?q=user/register",],
  'headers' => [
    'Content-type' => 'application/json',
  ],])->getBody()->getContents();
  $response = json_decode($request);
  $forumUrl = $response->forumAddress;
  $forumApiKey = $response->forumApiKey;
  \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_api_key', $forumApiKey)->save();
  \Drupal::service('config.factory')->getEditable('node.settings')->set('forum_url', $forumUrl)->save();
  if ($forumUrl) {
    if($forumEmbeddedOption == 1){
      createEmbeddedForum($response, $forumEmbeddedOption);
      updateForumData($forumUsername, $forumApiKey, $base_url . "/Forum");
    }else{
      createForumMenuUrl($response);
      updateForumData($forumUsername, $forumApiKey);
    }
  }else{
    $_SESSION['flag'] = 1;
    $urlRedirect      = $_SERVER['HTTP_REFERER'];
    header("location:" . $urlRedirect);
    exit();
  }
}
function createEmbeddedForum($response, $forumEmbeddedOption){
  $forumUrl   = $response->forumAddress;
  $query = \Drupal::entityQuery('node')->condition('title', 'Forum');
  $nids = $query->execute();
  if(empty($nids)){
    $node = Node::create(array('type' => 'page','title' => 'Forum','langcode' => 'en','uid' => '1','status' => 1));
    $node->body = array('format' => 'full_html', 'value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>');
    $node->save();
    $nodeId = $node->id();
    $menuLink = MenuLinkContent::create([
      'title' => Forum,
      'link' => ['uri' => 'internal:/node/' . $nodeId],
      'menu_name' => 'main',
      'expanded' => TRUE,
    ]);
    $menuLink->save();
    $path = \Drupal::service('path.alias_storage')->save("/node/" . $nodeId, "/forum", 'und');
    drupal_static_reset();
    drupal_flush_all_caches();
  }else{
    $getNodeArray = array_keys($nids);
    $getNodeID = array_shift($getNodeArray);
    $existNode = Node::load($getNodeID);
    $existNode->body = array('format' => 'full_html', 'value' => '<div id="wtEmbedCode"><script type="text/javascript" id="embedded_forum" src="' . $forumUrl . '/js/mb/embed.js"></script><noscript><a href="' . $forumUrl . '">Forum</a></noscript></div>');
    $existNode->save();
    db_update('menu_link_content_data')->fields(array('link__uri' => 'internal:/node/' . $getNodeID))->condition('title', 'Forum', '=')->execute();
    drupal_static_reset();
    drupal_flush_all_caches();
  }
}
    /*Update forum url and login/logout url on forum server configuration*/
function updateForumData($forumUsername, $forumAPI, $embedUrl = ""){
  global $base_url;
  $request = \Drupal::httpClient()->post(WT_SETTINGS_URL, [
    'verify' => true,
    'form_params' => ['action' => 'modifySSOURLs',
    'forumUsername' => $forumUsername,
    'forumApikey' => $forumAPI,
    'embed_page_url' => $embedUrl,
    'login_page_url' => $base_url . "/?q=user/register",
    'logout_page_url' => $base_url . "/?q=user/logout",
    'registration_url' => $base_url . "/?q=user/register",]
    ])->getBody()->getContents();
  }
      /*create url if embedded is disabled*/
function createForumMenuUrl($response){
    $forumUrl = $response->forumAddress;
    $menuExist = \Drupal::service('path.alias_storage')->aliasExists('/Forum', 'und');
    db_update('menu_link_content_data')->fields(array('link__uri' => $forumUrl))->condition('title', 'Forum', '=')->execute();
    drupal_static_reset();
    drupal_flush_all_caches();
}
