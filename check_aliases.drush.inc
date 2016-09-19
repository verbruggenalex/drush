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

  if (drupal_multilingual()) {

    // Don't proceed if we have entity translation or no translation at all.
    $translation_mechanism = variable_get('language_content_type_page', 0);
    if (!isset($translation_mechanism) || $translation_mechanism == ENTITY_TRANSLATION_ENABLED) {
      drush_log('No translation mechanism for this content type (page).', 'error');
      return;
    }

    $languages = language_list();
    $default_language = language_default();
    unset($languages[$default_language->language]);
    $translate_language = array_pop($languages);
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

    $tnid = $tnode->nid;
    $talias = drupal_get_path_alias('node/' . $tnid, $translate_language->language);

    // Check if translation alias matches source language alias.
    if ($alias == $talias) {
      drush_log('Alias ' . $alias . ' matches translation.', 'ok');
    } else {
      drush_log('Alias ' . $alias . ' does not match ' . $talias, 'notice');
    }

    // Check if alias of source language matches url generation.
    $url = url('node/' . $nid, array('language' => $default_language));

    if ($url == '/' . $default_language->language . '/' . $alias
      || $url == '/' . $alias . '_' . $default_language->language) {
      drush_log('Url of origin node generates alias :' . $url, 'ok');
    }
    else {
      drush_log('Url of origin node does not generate alias :' . $url, 'notice');
    }

    // Check if alias of source language matches url generation.
    $url = url('node/' . $tnid, array('language' => $translate_language));

    if ($url == '/' . $translate_language->language . '/' . $talias
      || $url == '/' . $talias . '_' . $translate_language->language) {
      drush_log('Url of translation node generates alias :' . $url, 'ok');
    }
    else {
      drush_log('Url of translation node does not generate alias :' . $url, 'notice');
    }

    node_delete($nid);
    node_delete($tnid);
  }
}