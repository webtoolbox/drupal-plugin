<?php

namespace Drupal\website_toolbox_forum\PageCache\RequestPolicy;

use Drupal\Core\PageCache\RequestPolicyInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Disables the internal page cache for visitors carrying forum SSO cookies.
 *
 * The forum login/logout image is rendered by hook_page_bottom() from the
 * forum SSO cookies, and the embedded forum URL is built from them as well.
 * The internal page cache keys entries on the request URI only, so a response
 * cached for a visitor without those cookies would be served to a visitor that
 * has them (missing the logout image, and embedding the forum with a stale
 * token). Requests carrying the cookies are therefore never served from - nor
 * stored in - the page cache.
 *
 * Returns NULL (no opinion) otherwise: an ALLOW here would override the
 * "undecided" verdict of Drupal's own policies for requests that carry a
 * session, making the internal page cache store authenticated responses -
 * which fails with "The database connection is not serializable" and 500s
 * the page.
 */
class ForumSsoCookies implements RequestPolicyInterface {

  /**
   * {@inheritdoc}
   */
  public function check(Request $request) {
    if ($request->cookies->has('forumLogoutToken') || $request->cookies->has('forumLoginUserid')) {
      return static::DENY;
    }
    return NULL;
  }

}
