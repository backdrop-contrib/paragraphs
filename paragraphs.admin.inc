<?php
/**
 * @file
 * Admin functions for the paragraphs module.
 */

/**
 * Page callback to show the type overview page.
 *
 * @return null|string
 *   Rendered table of types.
 *
 * @throws Exception
 */
function paragraphs_admin_type_overview() {
  $page = array();
  $types = paragraphs_type_load();
  $field_ui = module_exists('field_ui');

  $header = array(
    t('Icon'),
    t('Label'),
    t('Description'),
    t('Operations'),
  );
  $rows = array();
  $operations = array();
  foreach ($types as $type) {

    $type_url_str = strtr($type->bundle, array('_' => '-'));
    $label = '<div class="paragraphs-label">' . $type->name .
      '</div><div class="paragraphs-machine-name">(' . $type->bundle . ')</div>';
    if (!empty($type->icon)) {
      $iconfile = file_load($type->icon);
      $iconrender = theme('image_style', array(
        'style_name' => 'thumbnail',
        'uri' => $iconfile->uri,
        'attributes' => array(
          'class' => 'paragraphs-icon',
          ),
        ));
    }
    else {
      $iconrender = '';
    }
    $row = array(
      array(
        'data' => $iconrender,
      ),
      array(
        'data' => $label,
      ),
      array(
        'data' => $type->description,
      ),
    );

    if ($field_ui) {

      // Manage fields.
      $operations['manage_fields'] = array(
        'title' => t('Manage fields'),
        'href' => 'admin/structure/paragraphs/' . $type_url_str . '/fields',
      );

      // Display fields.
      $operations['manage_display'] = array(
        'title' => t('Manage display'),
        'href' => 'admin/structure/paragraphs/' . $type_url_str . '/display',
      );
    }

    // Manage type.
    $operations['edit_type'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/paragraphs/' . $type_url_str . '/edit',
    );
    // Delete type.
    $operations['delete_type'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/paragraphs/' . $type_url_str . '/delete',
    );

    $row['operations'] = array(
      'data' => array(
        '#type' => 'dropbutton',
        '#links' => $operations,
      ),
    );

    $rows[$type->bundle] = $row;
  }

  // Sort rows by type.
  ksort($rows);

  // Render paragraphs type table.
  $page['paragraphs_type_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No paragraph types have been defined yet.'),
  );

  return $page;
}

/**
 * Form to create or edit an paragraph type.
 */
function paragraphs_admin_type_form($form, &$form_state, $type = NULL) {

  if (!isset($type) && !$type) {
    // This is a new type
    $type = new stdClass();
    $type->name = '';
    $type->bundle = '';
    $type->locked = 0;
  }
  else {
    if (!$type) {
      backdrop_set_message(t('Could not load Paragraph type'), 'error');
      backdrop_goto('admin/structure/paragraphs');
    }
  }

  $form['#paragraphs_type'] = $type;

  $form['name'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $type->name,
    '#description' => t('Label for the Paragraph type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['icon'] = array(
    '#title' => t('Icon'),
    '#type' => 'managed_file',
    '#default_value' => (isset($type->icon)) ? $type->icon : '',
    '#upload_location' => 'public://paragraph_icons/',
    '#description' => t('Paragraph type icon.'),
    '#required' => FALSE,
    '#size' => 40,
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => (isset($type->description)) ? $type->description : '',
    '#description' => t('This text will be displayed on the <em>Add new paragraph</em> page.'),
    '#required' => FALSE,
    '#size' => 30,
  );

  if (!$type->locked) {

    $form['type'] = array(
      '#type' => 'machine_name',
      '#default_value' => $type->bundle,
      '#maxlength' => 32,
      '#disabled' => $type->locked,
      '#machine_name' => array(
        'exists' => 'paragraphs_type_load',
      ),
      '#description' => t('A unique machine-readable name for this Paragraph type. It must only contain lowercase letters, numbers, and underscores.'),
    );
  }

  $form['locked'] = array(
    '#type' => 'value',
    '#value' => $type->locked,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Paragraph type'),
    '#weight' => 40,
  );

  return $form;
}

/**
 * Form validation handler for paragraphs_admin_type_form().
 *
 * @see paragraphs_admin_type_form_submit()
 */
function paragraphs_admin_type_form_validate($form, &$form_state) {
  $type = new stdClass();
  $type->name = trim($form_state['values']['name']);

  if (!$form_state['values']['locked']) {
    $type->bundle = trim($form_state['values']['type']);
    // 'theme' conflicts with theme_node_form().
    // '0' is invalid, since elsewhere we check it using empty().
    if (in_array($type->bundle, array('0', 'theme'))) {
      form_set_error('type', t("Invalid machine-readable name. Enter a name other than %invalid.", array('%invalid' => $type->bundle)));
    }
  }

}

/**
 * Submit handler for paragraphs_admin_type_form().
 *
 * @see paragraphs_admin_type_form()
 */
function paragraphs_admin_type_form_submit($form, &$form_state) {

  $type = new stdClass();

  if (!$form_state['values']['locked']) {
    $type->bundle = trim($form_state['values']['type']);
  }
  else {
    $type->bundle = $form['#paragraphs_type']->bundle;
  }

  $type->locked = 1;

  $type->name = trim($form_state['values']['name']);
  $type->description = trim($form_state['values']['description']);
  $type->icon = trim($form_state['values']['icon']);

  if (!empty($type->icon)) {
    // Make file permanent
    $file = file_load($form_state['values']['icon']);
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);
    file_usage_add($file, 'paragraphs', 'paragraphs_type_icon', $form_state['values']['icon']);
  }

  $variables = $form_state['values'];

  // Remove everything that's been saved already - whatever's left is assumed
  // to be a persistent variable.
  foreach ($variables as $key => $value) {
    if (isset($type->$key)) {
      unset($variables[$key]);
    }
  }

  unset($variables['form_token'], $variables['op'], $variables['submit'], $variables['delete'], $variables['reset'], $variables['form_id'], $variables['form_build_id']);


  $status = paragraphs_type_save($type);

  $t_args = array('%name' => $type->name);

  if ($status == SAVED_UPDATED) {
    backdrop_set_message(t('The Paragraph type %name has been updated.', $t_args));
  }
  elseif ($status == SAVED_NEW) {
    backdrop_set_message(t('The Paragraph type %name has been added.', $t_args));
    watchdog('paragraphs', 'Added Paragraph type %name.', $t_args, WATCHDOG_NOTICE, l(t('view'), 'admin/structure/paragraphs'));
  }

  $form_state['redirect'] = 'admin/structure/paragraphs';
  return;
}

/**
 * Menu callback; delete a single paragraph type
 *
 * @ingroup forms
 */
function paragraphs_admin_type_delete_form($form, &$form_state, $type) {
  if (!$type) {
    backdrop_set_message(t('Could not load Paragraph type'), 'error');
    backdrop_goto('admin/structure/paragraphs');
  }

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type->bundle,
  );
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $type->name,
  );

  $message = t('Are you sure you want to delete the Paragraph type %type?', array('%type' => $type->name));
  $caption = '<p>' . t('This action cannot be undone. Content using this Paragraph type will be broken.') . '</p>';

  return confirm_form($form, filter_xss_admin($message), 'admin/structure/paragraphs', filter_xss_admin($caption), t('Delete'));
}

/**
 * Process paragraph type delete confirm submissions.
 *
 * @see paragraphs_admin_type_delete_form()
 */
function paragraphs_admin_type_delete_form_submit($form, &$form_state) {
  paragraphs_type_delete($form_state['values']['type']);

  $t_args = array('%name' => $form_state['values']['name']);
  backdrop_set_message(t('The Paragraph type %name has been deleted.', $t_args));
  watchdog('paragraphs', 'Deleted Paragraph type %name.', $t_args, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/structure/paragraphs';
  return;
}
