# Test cases — Drupal 11 plugin line (`enh-drupal11-compatibility`, commit bbe5142)

Scope: the whole change since master (one commit, one new module folder `drupal11.x/` + README).
Repo `webtoolbox/drupal-plugin` has no `.kilo/commands/test-cases-since-last-qa.md` spec, so the
case table below uses the house shape (ID, description, steps, expected result, grouped by feature).

## Test Environment Setup

1. Drupal 11 site (Drupal 11.4.13 recommended-project, PHP 8.5, own database) reachable at a URL of
   your choice; drush available (`vendor/bin/drush`).
2. Copy `drupal11.x/` into `web/modules/custom/website_toolbox_forum/` (the `info.yml` must sit at
   that folder root).
3. `drush pm:enable website_toolbox_forum -y` then `drush cr`.
4. A Website Toolbox forum whose admin username/password you know, and a forum that ordinary members
   can register on. On a beta the plugin's settings URL must be pointed at the beta host for the sync
   form (`src/Form/WebsitetoolboxSync.php` → `WT_SETTINGS_URL`), which is a test-local edit and must
   NOT be committed.
5. The forum site must be able to reach the forum host and the API stage (`getApiBaseUrl()` resolves a
   `betaN` forum to its own `betaN` stage).
6. Record the forum's own SSO URL columns, its `plugin` column, the member count, the user-group list
   and any required-field setting BEFORE the run, so they can be restored afterwards.

## Cases

### A. Install / version gate

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| A1 | Drupal 10 folder must be refused by Drupal 11 | Copy `drupal10.x/` in as the module, run `drush pm:enable website_toolbox_forum -y` | Enable FAILS: "module 'website_toolbox_forum' is incompatible with this version of Drupal core" |
| A2 | Drupal 11 folder enables | Copy `drupal11.x/` in as the module, run `drush pm:enable website_toolbox_forum -y` | `[success] Module website_toolbox_forum has been installed`; the module shows as enabled |
| A3 | Version declaration | Read `drupal11.x/website_toolbox_forum.info.yml` | `core_version_requirement: ^11` |
| A4 | Other version folders untouched | `git diff --name-only <master>...<branch>` | Only `drupal11.x/**` and `README.md` appear; `drupal7.x/`, `drupal8.x-9.x/`, `drupal10.x/` do not |

### B. Settings sync round-trip

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| B1 | Admin can open the settings form | Log in as an admin, open `/admin/website-toolbox-forum/development/configuration?forumSettingOption=1` | 200, form renders with the username/password fields |
| B2 | Sync submits | POST the form with the forum username + password, embedded ON, role sync ON | 302 redirect; no error message |
| B3 | Drupal side is configured | `drush config:get node.settings` | `forum_url` and `forum_api_key` populated; `forum_username` stored; embedded and role-sync both 1 |
| B4 | Forum page created | List nodes | One `page` node titled "Forum" |
| B5 | Alias and menu link created | List path aliases and menu links | `/node/<nid> => /forum`, and a main-menu link titled "Forum" |
| B6 | Forum side is rewritten | Read the forum's own settings row (`settings` table, forum uid) | `embed_url`, `login_page_url`, `logout_page_url`, `registration_url` point at the Drupal site and `plugin` = drupal |

### C. Embedded forum page

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| C1 | Forum page renders the embed | Open `/forum` as a guest | 200 and the page source contains the forum's `/js/mb/embed.js` |
| C2 | Token appended when signed in | Open `/forum` while signed in | The embed script src carries `?authtoken=…` |
| C3 | No content type on the site | On a fresh Drupal 11 site with no content type, run B2 then open `/forum` | The `page` type (with body field + displays) is created on demand; `/forum` returns 200 and renders the embed — no error page |

### D. Signup

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| D1 | Registration creates the Drupal account | Register a new account at `/user/register` | 303 and the account exists |
| D2 | Matching forum member created | Read the forum's member table for the new address | A member row exists for that username/email; member count grew by one |

### E. SSO sign-in / sign-out

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| E1 | Sign-in issues the forum cookies | Log in as a registered member | 303; `forumLoginUserid` (the forum member id) and `forumLogoutToken` cookies set |
| E2 | Sign-in pixel | Open any page while signed in | Page contains `register/dologin?authtoken=…` |
| E3 | Sign-out pixel for a departed session | Request the site carrying ONLY `forumLogoutToken` | Page contains `register/logout?authtoken=…` and all three forum cookies are expired in the response |
| E4 | No stale token in the page cache | Request the same page with and without the forum cookie | The cookie-carrying request is never served from the page cache |

### F. Role sync

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| F1 | Create a role | Create a role with role sync ON | A forum user group with the role's label is created |
| F2 | Rename a role | Change the role's label | The forum user group's title follows |
| F3 | Delete a role | Delete the role | The forum user group is removed |

### G. Uninstall lifecycle

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| G1 | Uninstall cleans up | `drush pm:uninstall website_toolbox_forum -y` | The forum page, its alias and its menu link are gone; every `forum_*` key is cleared |
| G2 | Re-enable works | `drush pm:enable website_toolbox_forum -y` | Enables again cleanly |

### H. Log hygiene

| ID | Description | Steps | Expected result |
|----|-------------|-------|-----------------|
| H1 | No module-originated entries | Run A–G, then list the site's log entries | Zero entries of type php from the module; only session-opened info lines and the expected "no mailer" mail error |

## Questionnaire

1. Did Drupal 11 refuse the Drupal 10 folder, and accept the Drupal 11 folder? — yes / no
2. Did the settings sync save the forum address and key, and rewrite the forum's own SSO URLs? — yes / no
3. Did `/forum` render the embedded forum, with the authtoken when signed in? — yes / no
4. Did a Drupal signup create the matching forum member? — yes / no
5. Did sign-in set both forum cookies and the sign-in pixel, and did a departed session get the sign-out pixel and cookie expiry? — yes / no
6. Did role create / rename / delete sync to the forum user groups? — yes / no
7. Did uninstall clean up and did the module re-enable? — yes / no
8. Any module-originated log entries after the round? — none / list them
