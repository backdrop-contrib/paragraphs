<?php

/**
 * @file
 * Admin functions for the paragraphs module.
 */

/**
 * Page callback to show the bundle overview page.
 *
 * @return null|string
 *   Rendered table of bundles.
 *
 * @throws Exception
 */
function paragraphs_admin_bundle_overview() {
  $page = array();
  $bundles = paragraphs_bundle_load();
  $field_ui = module_exists('field_ui');

  $header = array(
    t('Label'),
    t('Machine Name'),
    t('Operations'),
  );
  $rows = array();
  $operations = array();
  foreach ($bundles as $bundle) {

    $type_url_str = strtr($bundle->bundle, array('_' => '-'));
    $row = array(
      array(
        'data' => $bundle->name,
      ),
      array(
        'data' => $bundle->bundle,
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

    // Manage bundle.
    $operations['edit_type'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/paragraphs/' . $type_url_str . '/edit',
    );
    // Delete bundle.
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

    $rows[$bundle->bundle] = $row;
  }

  // Sort rows by bundle.
  ksort($rows);

  // Render paragraphs bundle table.
  $page['paragraphs_bundle_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No paragraph types have been defined yet.'),
  );

  return $page;
}

/**
 * Returns HTML for a paragraphs bundle description.
 *
 * @param array $variables
 *   An array with one item, containing the paragraphs bundle object.
 *   $variables['bundle'] is an object with the following properties:
 *   - name: The human-readable name of the bundle.
 *   - bundle: The machine name of the bundle.
 *   - label: The bundle's label when displayed in edit forms.
 *   - description: A text description of the bundle.
 *
 * @see paragraphs_admin_bundle_overview()
 * @ingroup themeable
 */
function theme_paragraphs_admin_overview(array $variables) {
  $bundle = $variables['bundle'];

  $output = check_plain($bundle->name);
  $output .= ($bundle->name != $bundle->label) ? ' [<em>' . check_plain($bundle->label) . '</em>]' : '';
  $output .= ' <small>' . t('(Machine name: @bundle)', array('@bundle' => $bundle->bundle)) . '</small>';
  $output .= !empty($bundle->description) ? '<div class="description">' . filter_xss_admin($bundle->description) . '</div>' : '';

  return $output;
}

/**
 * Form to create or edit an paragraphs bundle.
 *
 * @param array $form
 *   The form structure array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param object $bundle
 *   The bundle.
 *
 * @return array
 *   The form structure array.
 */
function paragraphs_admin_bundle_form(array $form, array &$form_state, $bundle = NULL) {

  if (!isset($bundle) && !$bundle) {
    // This is a new bundle.
    $bundle = new stdClass();
    $bundle->name = '';
    $bundle->bundle = '';
    $bundle->label = '';
    $bundle->description = '';
    $bundle->locked = 0;
  }
  else {
    if (!$bundle) {
      backdrop_set_message(t('Could not load Paragraph type'), 'error');
      backdrop_goto('admin/structure/paragraphs');
    }
  }

  $form['#paragraphs_bundle'] = $bundle;

  $form['name'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $bundle->name,
    '#description' => t('Label for the Paragraph type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['bundle'] = array(
    '#type' => 'machine_name',
    '#default_value' => $bundle->bundle,
    '#maxlength' => 32,
    '#disabled' => $bundle->locked,
    '#machine_name' => array(
      'exists' => 'paragraphs_bundle_load',
    ),
    '#description' => t('A unique machine-readable name for this Paragraph type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $bundle->label,
    '#description' => t('The label for this bundle as it will appear to users on edit forms. Defaults to bundle name if left empty.'),
    '#size' => 30,
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $bundle->description,
    '#description' => t('Describe this bundle. The text will be displayed on the Paragraphs admin overview page.'),
  );

  $form['locked'] = array(
    '#type' => 'value',
    '#value' => $bundle->locked,
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
 * Form validation handler for paragraphs_admin_bundle_form().
 *
 * @param array $form
 *   The form structure array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @see paragraphs_admin_bundle_form_submit()
 */
function paragraphs_admin_bundle_form_validate(array $form, array &$form_state) {
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

/**
 * Submit handler for paragraphs_admin_bundle_form().
 *
 * @param array $form
 *   The form structure array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @see paragraphs_admin_bundle_form()
 */
function paragraphs_admin_bundle_form_submit(array $form, array &$form_state) {

  $bundle = new stdClass();

  if (!$form_state['values']['locked']) {
    $bundle->bundle = trim($form_state['values']['bundle']);
  }
  else {
    $bundle->bundle = $form['#paragraphs_bundle']->bundle;
  }

  $bundle->locked = 1;

  $bundle->name = trim($form_state['values']['name']);

  // Set bundle label equal to name if empty.
  $bundle->label = empty($form_state['values']['label']) ? $bundle->name : trim($form_state['values']['label']);

  $bundle->description = trim($form_state['values']['description']);

  $variables = $form_state['values'];

  // Remove everything that's been saved already - whatever's left is assumed
  // to be a persistent variable.
  foreach ($variables as $key => $value) {
    if (isset($bundle->$key)) {
      unset($variables[$key]);
    }
  }

  unset($variables['form_token'], $variables['op'], $variables['submit'], $variables['delete'], $variables['reset'], $variables['form_id'], $variables['form_build_id']);

  $status = paragraphs_bundle_save($bundle);

  $t_args = array('%name' => $bundle->name);

  if ($status == SAVED_UPDATED) {
    backdrop_set_message(t('The Paragraph type %name has been updated.', $t_args));
  }
  elseif ($status == SAVED_NEW) {
    backdrop_set_message(t('The Paragraph type %name has been added.', $t_args));
    watchdog('paragraphs', 'Added Paragraph type %name.', $t_args, WATCHDOG_NOTICE, l(t('view'), 'admin/structure/paragraphs'));
  }

  $form_state['redirect'] = 'admin/structure/paragraphs';
}

/**
 * Menu callback to delete a single paragraph bundle.
 *
 * @param array $form
 *   The form structure array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param object $bundle
 *   The bundle.
 *
 * @ingroup forms
 */
function paragraphs_admin_bundle_delete_form(array $form, array &$form_state, $bundle) {
  if (!$bundle) {
    backdrop_set_message(t('Could not load Paragraph type'), 'error');
    backdrop_goto('admin/structure/paragraphs');
  }

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $bundle->bundle,
  );
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $bundle->name,
  );

  $message = t('Are you sure you want to delete the Paragraph type %bundle?', array('%bundle' => $bundle->name));
  $caption = '<p>' . t('This action cannot be undone. Content using this Paragraph type will be broken.') . '</p>';

  return confirm_form($form, filter_xss_admin($message), 'admin/structure/paragraphs', filter_xss_admin($caption), t('Delete'));
}

/**
 * Process and confirm paragraphs bundle deletion.
 *
 * @param array $form
 *   The form structure array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @see paragraphs_admin_bundle_delete_form()
 */
function paragraphs_admin_bundle_delete_form_submit(array $form, array &$form_state) {
  paragraphs_bundle_delete($form_state['values']['type']);

  $t_args = array('%name' => $form_state['values']['name']);
  backdrop_set_message(t('The Paragraph type %name has been deleted.', $t_args));
  watchdog('paragraphs', 'Deleted Paragraph type %name.', $t_args, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/structure/paragraphs';
}

/**
 * Separate paragraphs_item edit form
 */
function paragraphs_dialog_edit($form, &$form_state, $paragraphs_item) {
  if (!$paragraphs_item) {
    backdrop_not_found();
  }
  $bundle = paragraphs_bundle_load($paragraphs_item->bundle);
  backdrop_set_title(t('Edit !title paragraph', array('!title' => $bundle->name)));

  $form['paragraphs_item'] = array(
    '#type' => 'value',
    '#value' => $paragraphs_item,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 10000,
    '#value' => t('Save'),
    '#ajax' => array(
      'callback' => 'paragraphs_dialog_save_close',
      'effect' => 'fade',
      //'wrapper' => $element['#wrapper_id'],
    ),
  );
  /*
  $form['actions']['remove_button'] = array(
    '#type' => 'submit',
    '#weight' => 10000,
    '#value' => t('Remove'),
    '#submit' => array('paragraphs_dialog_delete_confirm'),
    '#access' => entity_access('delete', 'paragraphs_item', $paragraphs_item),
    '#attributes' => array(
      'class' => array('button-danger'),
    ),
  );
  */
  field_attach_form('paragraphs_item', $paragraphs_item, $form, $form_state);

  // Get the top-level host entity.
  $item = $paragraphs_item;
  $host = NULL;
  while (method_exists($item, 'hostEntity')) {
    $host = $item->hostEntity();
    $host_entity_type = $item->hostEntityType();
    $host_entity_id = $item->hostEntityId();
    $host_entity_bundle = $item->hostEntityBundle();
    $item = $host;
  }
  // Only show revisioning options if our host is definitely revisioned.
  $show_revision_options = FALSE;
  $use_revisions = FALSE;
  if ($host_entity_type == 'node') {
    $show_revision_options = config_get('node.type.' . $host_entity_bundle, 'settings.revision_enabled');
    $use_revisions = config_get('node.type.' . $host_entity_bundle, 'settings.revision_default');
  }
  if ($show_revision_options) {
    $form['additional_settings'] = array(
      '#type' => 'vertical_tabs',
      '#weight' => 99,
    );
    $form['revision_information'] = array(
      '#type' => 'fieldset',
      '#title' => t('Revision information'),
      '#collapsible' => TRUE,
      // Collapsed by default when "Create new revision" is unchecked.
      '#collapsed' => !$use_revisions,
      '#group' => 'additional_settings',
      '#attributes' => array(
        'class' => array('node-form-revision-information'),
      ),
      '#attached' => array(
        'js' => array(backdrop_get_path('module', 'node') . '/js/node.js'),
      ),
      '#weight' => 20,
      '#access' => user_access('administer nodes'),
    );
    $form['revision_information']['revision'] = array(
      '#type' => 'checkbox',
      '#title' => t('Create new revision'),
      '#default_value' => $use_revisions,
      '#access' => user_access('administer nodes'),
    );
  }

  return $form;
}

/**
 * Validation function for entity form for validating the fields.
 */
function paragraphs_dialog_edit_validate($form, &$form_state) {
  field_attach_form_validate('paragraphs_item', $form_state['values']['paragraphs_item'], $form, $form_state);
}

/**
 * Submit function for edit entity form.
 */
function paragraphs_dialog_edit_submit($form, &$form_state) {
  $paragraphs_item = $form_state['values']['paragraphs_item'];
  field_attach_submit('paragraphs_item', $paragraphs_item, $form, $form_state);

  // Save a new revision of our host entity?
  $save_new_revision = !empty($form_state['values']['revision']);
  if ($save_new_revision) {
    $paragraphs_item->is_new_revision = TRUE;
  }

  // You get a new revision when you save a node, even if there are no changes.
  // Ensure that we do not save the host node if $save_new_revision is FALSE.
  $paragraphs_item->save(FALSE);
}

function paragraphs_dialog_save_close($form, &$form_state) {
  $paragraphs_item = $form_state['values']['paragraphs_item'];
  $bundle = paragraphs_bundle_load($paragraphs_item->bundle);
  // Close modal dialog upon submit.
  $commands = array(
    '#type' => 'ajax',
    '#commands' => array(),
  );
  $commands['#commands'][] = ajax_command_close_modal_dialog();
  
  // Refresh display of Paragraph that was edited.
  if (is_object($form_state['build_info']['args'][0]) && !empty($form_state['build_info']['args'][0]->item_id)) {
    $item_id = $form_state['build_info']['args'][0]->item_id;
    $revision_id = $form_state['build_info']['args'][0]->revision_id;
    $fresh_paragraph = paragraphs_item_revision_load($revision_id)->view();
    //$new_element = "<div class='paragraphs-item paragraphs-item-" . $item_id . "'>";
    //$new_element .= backdrop_render(paragraphs_separate_edit_links($item_id));
    $new_element = '<div class=\'messages status paragraphs-separate-edit-message\'>' . t('Paragraph !type has been updated.', array('!type' => $bundle->name)) . '</div>';
    $new_element .= backdrop_render($fresh_paragraph['paragraphs_item']);
    $new_element .= "</div>";
    $commands['#commands'][] = ajax_command_html('.paragraphs-item-' . $item_id . ' .content', $new_element);
    $commands['#commands'][] = array(
        'command' => 'paragraphs_separate_edit_message_hide',
        //'value1'  => 'My first value',
    );
  }
  return $commands;
}

/**
 * Paragraphs sort dialog popup.
 */
function paragraphs_dialog_sort($form, &$form_state, $paragraphs_item) {
  if (!$paragraphs_item) {
    backdrop_not_found();
  }
  dpm($paragraphs_item);
  $langcode = $paragraphs_item->langcode;
  $host_entity = $paragraphs_item->hostEntity();
  $field_name = $paragraphs_item->field_name;
  $element = $paragraphs_item->fieldInfo();
  $element['#instance'] = $paragraphs_item->instanceInfo();
  $items = $host_entity->{$field_name};
  dpm($element);

  $output = '';
  $instance = $element['#instance'];

  // If the field can hold more than one item, display it as a draggable table.
  if ($element['cardinality'] != 1) {
    $table_id = backdrop_html_id($element['field_name'] . '_values');
    $order_class = $element['field_name'] . '-' . $table_id . '-delta-order';

    $header = array(
      array(
        'data' => '<label>' . t('!title', array('!title' => $element['#instance']['settings']['title'])) . '</label>',
        'colspan' => 2,
        'class' => array('field-label'),
      ),
      t('Order'),
    );
    $rows = array();

    //usort($items, '_field_sort_items_value_helper');
    // Add the items as table rows.
    foreach ($items[$langcode] as $key => $item) {
      $item['#markup'] = 'Paragraph ' . $item['value'];
      $item['_weight']['#value'] = $key;
      $item['_weight']['#attributes']['class'] = array($order_class);
      //$delta_element = backdrop_render($item['_weight']);
      $cells = array(
        array(
          'data' => '',
          'class' => array('field-multiple-drag'),
        ),
        $item['#markup'],
        array(
          'data' => $key,
          'class' => array('delta-order'),
        ),
      );
      $class = !empty($paragraphs_item->bundle) ? array('draggable', backdrop_html_class('paragraphs_item_type_' . $paragraphs_item->bundle)) : array();
      $rows[] = array(
        'data' => $cells,
        'class' => $class,
      );
    }

    $field_content = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array(
        'id' => $table_id,
        'class' => array('field-multiple-table'),
      ),
    ));

    //backdrop_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }

  if (count($items[$langcode])) {
    $output .= '<div class="form-item">';
    //$output .= print_r($items[$langcode], TRUE);
    $output .= $field_content;
    $output .= $element['#instance']['description'] ? '<div class="description">' . $element['#instance']['description'] . '</div>' : '';
    $output .= '</div>';
  }

  return array(
    array(
      '#markup' => $output,
    ),
  );
}


/**
 * Page callback: Form constructor for paragraph deletion confirmation form.
 *
 * @see paragraphs_menu()
 * @see paragraphs_dialog_delete_confirm_submit()
 */
function paragraphs_dialog_delete_confirm($form, &$form_state, $paragraphs_item = NULL) {
  //dpm($paragraphs_item);
  //dpm($form);
  //dpm($form_state);
  if (empty($paragraphs_item) && !empty($form['paragraphs_item'])) {
    $paragraphs_item = $form['paragraphs_item'];
  }
  $form['item_id'] = array(
    '#type' => 'value',
    '#value' => $paragraphs_item->item_id,
  );
  $bundle = paragraphs_bundle_load($paragraphs_item->bundle);
  $form['description'] = array(
    '#markup' => t('Are you sure you want to delete this !type? (This action cannot be undone.)', array('!type' => $bundle->name)),
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#attributes' => array('class' => array('button-danger')),
    '#ajax' => array(
      'callback' => 'paragraphs_dialog_delete_confirm_submit',
      'effect' => 'fade',
      //'wrapper' => $element['#wrapper_id'],
    ),
  );
  return $form;
}

/**
 * Form submission handler for aragraphs_dialog_delete_confirm().
 *
 * @see aragraphs_dialog_delete_confirm()
 */
function paragraphs_dialog_delete_confirm_submit($form, &$form_state) {
  /*
    //TODO: Remove the paragraph if the Remove button was clicked... (separate function/callback?)
    $form_state['triggering_element']['#value'] == 'Remove';
    $form_state['values']['paragraph_item']->item_id;

  if ($form_state['values']['confirm']) {
    $node = node_load($form_state['values']['nid']);
    node_delete($form_state['values']['nid']);
    watchdog('content', '@type: deleted %title.', array('@type' => $node->type, '%title' => $node->title));
    backdrop_set_message(t('@type %title has been deleted.', array('@type' => node_type_get_name($node), '%title' => $node->title)));
  }


    
    */
  // Close modal dialog upon submit.
  $commands = array(
    '#type' => 'ajax',
    '#commands' => array(),
  );
  $commands['#commands'][] = ajax_command_close_modal_dialog();
  // Delete paragraph item and refresh display.
  if (is_object($form_state['build_info']['args'][0]) && !empty($form_state['build_info']['args'][0]->item_id)) {
    $item_id = $form_state['build_info']['args'][0]->item_id;
    $paragraph = paragraphs_item_load($item_id);
    $paragraph->deleteRevision();
    //entity_delete_multiple('paragraphs_item', array($item_id));
    //dpm($paragraph);
    $commands['#commands'][] = ajax_command_replace('.paragraphs-item-' . $item_id, '<div class=\'messages status paragraphs-separate-edit-message\'>Paragraph has been deleted.</div>');
    $commands['#commands'][] = array(
        'command' => 'paragraphs_separate_edit_message_hide', // the name of your javascript callback
    );
  }
  return $commands;
}

function paragraphs_separate_edit_placeholder() {
  $form['placeholder'] = array(
    '#type' => 'item',
    '#title' => t('Placeholder'),
    '#markup' => 'This functionality has not been developed yet.',
  );
  return $form;
}