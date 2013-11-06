<?php

function paragraph_admin_bundle_overview() {
  $bundles = paragraph_bundle_load();

  $header = array(t('Bundle name'), array('data' => t('Operations'), 'colspan' => '4'));
  $rows = array();
  foreach ($bundles as $bundle_machine_name => $bundle) {
    $rows[] = array(
      array(
        'data' => $bundle->name . ' (' . $bundle->bundle . ')',
      ),
      array(
        'data' => l(t('manage fields'), 'admin/structure/paragraph/' . strtr($bundle->bundle, array('_' => '-')) . '/fields')
      ),
      array(
        'data' => l(t('manage display'), 'admin/structure/paragraph/' . strtr($bundle->bundle, array('_' => '-')) . '/display')
      ),
      array(
        'data' => l(t('edit bundle'), 'admin/structure/paragraph/' . strtr($bundle->bundle, array('_' => '-')) . '/edit')
      ),
      array(
        'data' => l(t('delete bundle'), 'admin/structure/paragraph/' . strtr($bundle->bundle, array('_' => '-')) . '/delete')
      ),
    );
  }
  if (empty($rows)) {
    $output = t('No paragraph bundles have been defined yet.');
  }
  else {
    // Sort rows by field name.
    ksort($rows);
    $output = theme('table', array('header' => $header, 'rows' => $rows));
  }
  return $output;
}

function paragraph_admin_bundle_form($form, &$form_state, $bundle = NULL) {

  if (!isset($bundle) && !$bundle) {
    // This is a new type. Node module managed types are custom and unlocked.
    $bundle = new stdClass();
    $bundle->name = '';
    $bundle->bundle = '';
    $bundle->locked = 0;
  } else {
    if(!$bundle) {
      drupal_set_message(t('Could not load bundle'), 'error');
      drupal_goto('admin/structure/paragraph');
    }
  }

  $form['#field_collection_bundle'] = $bundle;

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => $bundle->name,
    '#description' => t('The human-readable name of this bundle. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  if(!$bundle->locked) {

    $form['bundle'] = array(
      '#type' => 'machine_name',
      '#default_value' => $bundle->bundle,
      '#maxlength' => 32,
      '#disabled' => $bundle->locked,
      '#machine_name' => array(
        'exists' => 'paragraph_bundle_load',
      ),
      '#description' => t('A unique machine-readable name for this content type. It must only contain lowercase letters, numbers, and underscores.'),
    );
  }

  $form['locked'] = array(
    '#type' => 'value',
    '#value' => $bundle->locked,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Paragraph bundle'),
    '#weight' => 40,
  );

  return $form;
}

/**
 * Form validation handler for node_type_form().
 *
 * @see node_type_form_submit()
 */
function paragraph_admin_bundle_form_validate($form, &$form_state) {
  $bundle = new stdClass();
  $bundle->name = trim($form_state['values']['name']);

  if (!$form_state['values']['locked']) {
    $bundle->bundle = trim($form_state['values']['bundle']);
    // 'theme' conflicts with theme_node_form().
    // '0' is invalid, since elsewhere we check it using empty().
    if (in_array($bundle->bundle, array('0', 'theme'))) {
      form_set_error('type', t("Invalid machine-readable name. Enter a name other than %invalid.", array('%invalid' => $bundle->bundle)));
    }
  }

}

function paragraph_admin_bundle_form_submit($form, &$form_state) {

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  $bundle = new stdClass();

  if(!$form_state['values']['locked']) {
    $bundle->bundle = trim($form_state['values']['bundle']);
  } else {
    $bundle->bundle = $form['#field_collection_bundle']->bundle;
  }

  $bundle->locked = 1;

  $bundle->name = trim($form_state['values']['name']);

  $variables = $form_state['values'];

  // Remove everything that's been saved already - whatever's left is assumed
  // to be a persistent variable.
  foreach ($variables as $key => $value) {
    if (isset($bundle->$key)) {
      unset($variables[$key]);
    }
  }

  unset($variables['form_token'], $variables['op'], $variables['submit'], $variables['delete'], $variables['reset'], $variables['form_id'], $variables['form_build_id']);


  $status = paragraph_bundle_save($bundle);

  $t_args = array('%name' => $bundle->name);

  if ($status == SAVED_UPDATED) {
    drupal_set_message(t('The paragraph bundle %name has been updated.', $t_args));
  }
  elseif ($status == SAVED_NEW) {
    drupal_set_message(t('The content type %name has been added.', $t_args));
    watchdog('node', 'Added paragraph bundle %name.', $t_args, WATCHDOG_NOTICE, l(t('view'), 'admin/structure/paragraph'));
  }

  $form_state['redirect'] = 'admin/structure/paragraph';
  return;
}

/**
 * Menu callback; delete a single paragraph bundle
 *
 * @ingroup forms
 */
function paragraph_admin_bundle_delete_form($form, &$form_state, $bundle) {
  if(!$bundle) {
    drupal_set_message(t('Could not load bundle'), 'error');
    drupal_goto('admin/structure/paragraph');
  }

  $form['type'] = array('#type' => 'value', '#value' => $bundle->bundle);
  $form['name'] = array('#type' => 'value', '#value' => $bundle->name);

  $message = t('Are you sure you want to delete the paragraph bundle %bundle?', array('%bundle' => $bundle->name));
  $caption = '';
  $caption .= '<p>' . t('This action cannot be undone. Content using the bundle will be broken.') . '</p>';

  return confirm_form($form, $message, 'admin/structure/paragraph', $caption, t('Delete'));
}

/**
 * Process paragraph bundle delete confirm submissions.
 *
 * @see paragraph_admin_bundle_delete_form()
 */
function paragraph_admin_bundle_delete_form_submit($form, &$form_state) {
  paragraph_bundle_delete($form_state['values']['type']);

  $t_args = array('%name' => $form_state['values']['name']);
  drupal_set_message(t('The pargraph bundle %name has been deleted.', $t_args));
  watchdog('node', 'Deleted paragraph bundle %name.', $t_args, WATCHDOG_NOTICE);


  $form_state['redirect'] = 'admin/structure/paragraph';
  return;
}
