<?php

/**
 * @file
 * Paragraphs library module theme functions.
 */

/**
 * Process variables for paragraphs-library-item.tpl.php.
 */
function template_preprocess_paragraphs_library_item(&$variables) {
  $variables['view_mode'] = $variables['element']['#view_mode'];
  $paragraphs_library_item_type = $variables['element']['#entity_plus_type'];
  $variables['entity_type'] = $paragraphs_library_item_type;
  $paragraphs_library_item = $variables['element']['#entity'];
  $variables[$paragraphs_library_item_type] = $paragraphs_library_item;

  list(, , $bundle) = entity_extract_ids($paragraphs_library_item_type, $paragraphs_library_item);
  $variables['bundle'] = $bundle;

  // $paragraphs_library_item_id = entity_plus_id($paragraphs_library_item_type, $paragraphs_library_item);

  $variables['title'] = check_plain(entity_label($paragraphs_library_item_type, $paragraphs_library_item));

  $uri = entity_uri($paragraphs_library_item_type, $paragraphs_library_item);
  $variables['url'] = $uri && !empty($uri['path']) ? url($uri['path'], $uri['options']) : FALSE;

  if (isset($variables['element']['#page'])) {
    // If set by the caller, respect the page property.
    $variables['page'] = $variables['element']['#page'];
  }
  else {
    // Else, try to automatically detect it.
    $variables['page'] = $uri && !empty($uri['path']) && $uri['path'] == $_GET['q'];
  }

  $variables['content'] = array();
  $library_item_properties = [
    'title',
    'created',
    'changed',
    'uid',
  ];
  $fullpage = ($uri['path'] == current_path());
  foreach (element_children($variables['element']) as $key) {
    $element_type = (in_array($key, $library_item_properties)) ? 'property' : 'content';
    if ($element_type == 'property' && !$fullpage) {
      // Skip properties if not on the full page.
      continue;
    }
    $variables[$element_type][$key] = $variables['element'][$key];
  }
  if ($variables['view_mode'] == 'paragraphs_library_item') {
    $variables['url'] = false;
  }
  // Make the field variables available with the appropriate language.
  field_attach_preprocess($paragraphs_library_item_type, $paragraphs_library_item, $variables['content'], $variables);
}
