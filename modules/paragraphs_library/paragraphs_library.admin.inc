<?php

/**
 * Entity Form.
 */
function paragraphs_library_form($form, &$form_state, $paragraphs_library_item) {
  $form_state['paragraphs_library_item'] = $paragraphs_library_item;

  $form['title'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Title'),
    '#default_value' => $paragraphs_library_item->title,
  );

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $paragraphs_library_item->uid,
  );

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $paragraphs_library_item->type,
  );

  if (!(isset($paragraphs_library_item->is_new) && $paragraphs_library_item->is_new)) {
    $form['paragraphs_library_id'] = array(
      '#type' => 'value',
      '#value' => $paragraphs_library_item->paragraphs_library_id,
    );
  }

  field_attach_form('paragraphs_library_item', $paragraphs_library_item, $form, $form_state);

  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Paragraphs library item'),
    '#submit' => $submit + array('paragraphs_library_form_submit'),
  );

  // Show Delete button if we edit paragraphs_library.
  $paragraphs_library_id = entity_plus_id('paragraphs_library_item', $paragraphs_library_item);
  if (!empty($paragraphs_library_id) && paragraphs_library_access('edit', $paragraphs_library_item)) {
    $paragraphs_library_item_uri = $paragraphs_library_item->uri();
    $form['actions']['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => $paragraphs_library_item_uri['path'] . '/delete',
      '#options' => array(
        'query' => isset($_GET['destination']) ? backdrop_get_destination() : array(),
        'attributes' => array('class' => array('button', 'button-secondary', 'form-delete'))
      ),
      '#weight' => 15,
    );
  }

  $form['#validate'][] = 'paragraphs_library_form_validate';

  return $form;
}

function paragraphs_library_form_validate($form, &$form_state) {
  entity_form_field_validate('paragraphs_library_item', $form, $form_state);
}

/**
 * Entity submit handler.
 */
function paragraphs_library_form_submit($form, &$form_state) {
  $paragraphs_library_item = $form_state['paragraphs_library_item'];
  entity_form_submit_build_entity('paragraphs_library_item', $paragraphs_library_item, $form, $form_state);
  paragraphs_library_item_save($paragraphs_library_item);
  $paragraphs_library_item_uri = $paragraphs_library_item->uri();
  $form_state['redirect'] = $paragraphs_library_item_uri['path'];
  backdrop_set_message(t('Paragraphs library item  %title saved.', array('%title' => entity_label('paragraphs_library_item', $paragraphs_library_item))));
}

/**
 * Delete confirmation form.
 */
function paragraphs_library_delete_form($form, &$form_state, $paragraphs_library_item) {
  $form_state['paragraphs_library_item'] = $paragraphs_library_item;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['paragraphs_library_type_id'] = array('#type' => 'value', '#value' => entity_plus_id('paragraphs_library_item' ,$paragraphs_library_item));
  $paragraphs_library_item_uri = $paragraphs_library_item->uri();
  return confirm_form($form,
    t('Are you sure you want to delete Paragraphs library item %title?', array('%title' => entity_label('paragraphs_library_item', $paragraphs_library_item))),
    $paragraphs_library_item_uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function paragraphs_library_delete_form_submit($form, &$form_state) {
  $paragraphs_library_item = $form_state['paragraphs_library_item'];
  paragraphs_library_item_delete($paragraphs_library_item);

  backdrop_set_message(t('Paragraphs library item  %title deleted.', array('%title' => entity_label('paragraphs_library_item', $paragraphs_library_item))));

  $form_state['redirect'] = '<front>';
}

/**
 * Delete confirmation for multiple paragraphs_library entities through action and bulk operation
 */
function paragraphs_library_multiple_delete_confirm($form, &$form_state) {
  if (isset($form_state['paragraphs_library_ids'])) {
    $paragraphs_library_ids = $form_state['paragraphs_library_ids'];
  }
  elseif (isset($_SESSION['paragraphs_library_delete_action']['timestamp']) && (REQUEST_TIME - $_SESSION['paragraphs_library_delete_action']['timestamp'] < 6000)) {
    $paragraphs_library_ids = $_SESSION['paragraphs_library_delete_action']['paragraphs_library_ids'];
    $form_state['paragraphs_library_ids'] = $paragraphs_library_ids;
    $form_state['cache'] = TRUE;
    unset($_SESSION['paragraphs_library_delete_action']);
  }
  else {
    $paragraphs_library_ids = array();
  }

  $form['#tree'] = TRUE;

  if (empty($paragraphs_library_ids)) {
    $destination = isset($_GET['destination']) ? $_GET['destination'] : '<front>';
    $form['empty']['#markup'] = '<p>' . t('Return to the <a href="!url">front page</a>.', array('!url' => url($destination))) . '</p>';
    backdrop_set_message(t('No Paragraphs library items have been selected for deletion.'), 'error');
    return $form;
  }

  $form['paragraphs_library_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  $rows = db_query('SELECT paragraphs_library_id, title FROM {paragraphs_library} WHERE paragraphs_library_id IN (:paragraphs_library_ids)', array(':paragraphs_library_ids' => $paragraphs_library_ids))->fetchAllKeyed();
  foreach ($rows as $paragraphs_library_id => $title) {
    $form['paragraphs_library_entities'][$paragraphs_library_id] = array(
      '#type' => 'hidden',
      '#value' => $paragraphs_library_id,
    );
    $form['paragraphs_library_list']['#items'][] = check_plain($title);
  }

  $confirm_question = format_plural(count($rows), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, '<front>', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for paragraphs_library_multiple_delete_confirm().
 */
function paragraphs_library_multiple_delete_confirm_submit($form, &$form_state) {
  paragraphs_library_item_delete_multiple(array_keys($form_state['values']['paragraphs_library_entities']));
  cache_clear_all();
  $count = count($form_state['values']['paragraphs_library_entities']);
  watchdog('paragraphs_library', 'Deleted @count basic entities.', array('@count' => $count));

  backdrop_set_message(format_plural($count, 'Deleted 1 paragraphs_library.', 'Deleted @count basic entities.'));
  $form_state['redirect'] = '<front>';
}

/**
 * Displays a simple page with all available Paragraphs library types, linked to
 * the administration page.
 */
function paragraphs_library_type_view() {
  $all_types = paragraphs_library_type_load();
  $items = array();
  foreach ($all_types as $type_info) {
    $items[] = l($type_info->label, 'admin/structure/paragraphs/library/types/' . $type_info->type);
  }
  return theme('item_list', array('items' => $items, 'title' => t('Available types')));
}

/**
 * Helper to save a Paragraphs library type.
 */
function paragraphs_library_type_save(ParagraphsLibraryType $paragraphs_library_type) {
  // Save the Paragraphs library type configuration.
  $config = config('paragraphs_library.type.' . $paragraphs_library_type->type);
  $config->set('label', $paragraphs_library_type->label);
  $config->save();

  // Notify Field API of the new bundle.
  field_attach_create_bundle('paragraphs_library_item', $paragraphs_library_type->type);
}

/**
 * Form to create a new Paragraphs library item type.
 */
function paragraphs_library_type_create_form($form, &$form_state, ?Object $paragraphs_library_type = NULL) {
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Paragraphs library type name'),
    '#required' => TRUE,
    '#default_value' => !empty($paragraphs_library_type->label) ? $paragraphs_library_type->label : '',
  );
  $form['type'] = array(
    '#type' => 'machine_name',
    '#title' => 'Paragraphs library type machine name',
    '#maxlegth' => 21,
    '#required' => TRUE,
    '#machine_name' => array(
      'exists' => 'paragraphs_library_type_exists',
      'source' => array('label'),
    ),
    '#disabled' => !empty($paragraphs_library_type),
  );
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Create'),
    ),
  );
  return $form;
}

/**
 * Callback to be sure the bundle machine name is unique.
 */
function paragraphs_library_type_exists($value) {
  $names = config_get_names_with_prefix('paragraphs_library.type.' . $value);
  return !empty($names);
}

/**
 * Paragraphs library type form submit handler.
 */
function paragraphs_library_type_create_form_submit($form, &$form_state) {
  $config = config('paragraphs_library.type.' . $form_state['values']['type']);
  $config->set('label', $form_state['values']['label']);
  $config->set('type', $form_state['values']['type']);
  $config->save();

  // Notify Field API of the new bundle.
  field_attach_create_bundle('paragraphs_library_item', $form_state['values']['type']);

  backdrop_set_message(t('Paragraphs library type %label saved.', array('%label' => $form_state['values']['label'])));
  $form_state['redirect'] = 'admin/structure/paragraphs/library/types';
  menu_rebuild();
}

/**
 * Title callback for Paragraphs library item type administration pages.
 *
 * This is used by the Field API to show the name of the bundle when we click on
 * Manage fields or Manage displays
 */
function paragraphs_library_type_page_title($paragraphs_library_type) {
  return $paragraphs_library_type->label;
}

/**
 * Displays a simple page with the bundle information for a specific bundle.
 *
 * This page serves as a base path for Field API to display the Manage fields
 * and Manage displays tabs for the bundle.
 */
function paragraphs_library_type($paragraphs_library_type) {
  backdrop_set_title(t('Paragraphs library type information for @paragraphs_library_type', array('@paragraphs_library_type' => $paragraphs_library_type->label)));
  $output['bundle_label'] = array(
    '#markup' => '<div><strong>Paragraphs library type label: </strong>' . $paragraphs_library_type->label,
  );
  $output['bundle_machine_name'] = array(
    '#markup' => '<div><strong>Paragraphs library type machine name: </strong>' . $paragraphs_library_type->type,
  );
  return $output;
}
