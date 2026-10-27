<?php

/**
 * @file
 * Hook implementations for i18n_string module hooks.
 */

/**
 * Implements hook_i18n_string_info().
 */
function paragraphs_i18n_i18n_string_info() {
  $groups['paragraphs'] = array(
    'title' => 'Paragraphs',
    'description' => 'Supports configurable paragraph translations.',
    'format' => FALSE,
    'list' => TRUE,
  );
  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function paragraphs_i18n_i18n_object_info() {
  $info['paragraphs_bundle'] = array(
    'title' => 'Paragraphs bundle',
    'key' => 'bundle',
    'load callback' => 'paragraphs_bundle_load',
    // Placeholders for automatic paths.
    'placeholders' => array(
      '%paragraphs_bundle' => 'bundle',
    ),

    // Properties for string translation.
    'string translation' => array(
      'textgroup' => 'paragraphs',
      'type' => 'paragraphs_bundle',
      'properties' => array(
        'name' => t('Name'),
        'label' => t('Label'),
        'description' => t('Description'),
      ),
      'translate path' => 'admin/structure/paragraphs/%paragraphs_bundle/translate/%i18n_language',
    ),
    'edit path' => 'admin/structure/paragraphs/%paragraphs_bundle/configure',
    'translate tab' => 'admin/structure/paragraphs/%paragraphs_bundle/translate',
  );

  return $info;
}

/**
 * Implements hook_i18n_string_objects().
 */
function paragraphs_i18n_i18n_string_objects($type) {
  if ($type == 'paragraphs_bundle') {
    return paragraphs_bundle_load();
  }
}
