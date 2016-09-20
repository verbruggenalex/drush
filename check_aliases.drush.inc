<?php


/**
 * @file
 * Drush implementation for to check aliases for content_translation.
 */

/**
 * Implementats hook_drush_command().
 */
function check_aliases_drush_command() {
  $items = array();
  $items['check-ct-aliases'] = array(
    'description' => 'Create node and translation and compare aliases.',
  );
  return $items;
}

/**
 * Drush command callback.
 */
function drush_check_aliases_check_ct_aliases() {

  // Prepare needed languages.
  $languages = language_list();
  $default_language = language_default();
  unset($languages[$default_language->language]);
  $translate_language = array_pop($languages);

  // Get url alias patterns.
  $node_pattern = variable_get('pathauto_node_pattern', '');
  $page_pattern = variable_get('pathauto_node_page_pattern', '');
  $page_default_pattern = variable_get('pathauto_node_page_' . $default_language->language . '_pattern', '');
  $page_translate_pattern = variable_get('pathauto_node_page_' . $translate_language->language . '_pattern', '');

  // If we have only one language and there is a pattern set for the page content
  // type. Then start the checks on our aliases.
  if (count($languages) > 1 && (!empty($node_pattern) || !empty ($page_pattern) || !empty($page_default_pattern) || !empty($page_translate_pattern))) {

    // Do not proceed if we have entity translation or no translation  at all.
    $translation_mechanism = variable_get('language_content_type_page', 0);
    if (!isset($translation_mechanism) || $translation_mechanism == ENTITY_TRANSLATION_ENABLED) {
      return;
    }

    // Set the title
    $title = 'Alias for page';

    // Create a node.
    $node = new stdClass();
    $node->type = 'page';
    node_object_prepare($node);
    $node->title = $title;
    $node->uid = 1;
    $node->status = 1;
    $node->language = $default_language->language;
    node_save($node);
    $node->tnid = $node->nid;
    node_save($node);

    // Set nid and get alias.
    $nid = $node->nid;
    $alias = drupal_get_path_alias('node/' . $nid, $default_language->language);

    // Create a translation of the node.
    $tnode = new stdClass();
    $tnode->type = 'page';
    node_object_prepare($node);
    $tnode->title = $title;
    $tnode->uid = 1;
    $tnode->status = 1;
    $tnode->language = $translate_language->language;
    $tnode->tnid = $node->nid;
    node_save($tnode);

    // Set nid and get alias for translation.
    $tnid = $tnode->nid;
    $talias = drupal_get_path_alias('node/' . $tnid, $translate_language->language);

    // Check if translation alias matches source language alias.
    if ($alias == $talias) {
      drush_print(dt('OK: Alias ' . $alias . ' matches translation.'));
    } else {
      drush_print(dt('NOK: Alias ' . $alias . ' does not match ' . $talias));
    }

    // Check if alias of source language matches url generation.
    $url = url('node/' . $nid, array('language' => $default_language));

    if (strpos($url, '/' . $alias) !== false
      || strpos($url, '/' . $alias . '_' . $default_language->language) !== false) {
      drush_print(dt('OK: Url of origin node generates alias :' . $url));
    }
    else {
      drush_print(dt('NOK: Url of origin node does not generate alias :' . $url));
    }

    // Check if alias of source language matches url generation.
    $url = url('node/' . $tnid, array('language' => $translate_language));

    if (strpos($url, '/' . $translate_language->language . '/' . $alias) !== false
      || strpos($url, '/' . $alias . '_' . $translate_language->language) !== false) {
      drush_print(dt('OK: Url of translation node generates alias :' . $url));
    }
    else {
      drush_print(dt('NOK: Url of translation node does not generate alias :' . $url));
    }

    // Cleanup the created nodes.
    node_delete($nid);
    node_delete($tnid);
  }
}