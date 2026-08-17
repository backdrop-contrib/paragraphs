<?php
/**
 * @file
 */

/**
 * Add an interface to select a Layout Paragraphs template from the options.
 */
function layout_paragraphs_select_template_form($form, &$form_state, $layout) {
  $build = [];
  $template_options = _layout_paragraphs_layout_get_available_templates($layout->name);

  $build['#attributes'] = array(
    'class' => array('layout-paragraphs-settings-form'),
  );

  $intro_text = '';
  $intro_text .= '<p class="messages warning description">' . t('If you have unsaved content in fields that are outside of the layout, you may wish to close this dialog and save first to ensure you do not lose those changes.') . '</div>';
  $intro_text .= t(
    '<p class="description">The currently selected layout template is "@template". You may select a different layout template from the options below. Existing content within the current layout will need to be shifted around to fit into the new template.</p>',
    array('@template' => $template_options['active']->title)
  );
  $build['intro'] = array(
    '#markup' => $intro_text,
  );
  $build['layout_paragraphs_layout_name'] = array(
    '#type' => 'hidden',
    '#value' => $layout->name,
  );
  $build['layout_paragraphs_templates'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-paragraphs-templates'),
    ),
  );
  $build['layout_paragraphs_templates']['active'] = array(
    '#title' => t('Active template'),
    '#type' => 'radios',
    '#default_value' => $template_options['active']->name,
    '#options' => array(
      $template_options['active']->name => $template_options['active']->title,
    ),
    '#wrapper_attributes' => array(
      'class' => array('layout-active'),
    ),
    '#disabled' => TRUE,
  );
  $build['layout_paragraphs_templates']['active'][$template_options['active']->name]['#description'] = $template_options['active']->preview;
  $build['layout_paragraphs_templates']['available'] = array(
    '#title' => t('Other available templates'),
    '#type' => 'radios',
    '#default_value' => '',
    '#options' => array(),
    '#wrapper_attributes' => array(
      'class' => array('layout-options'),
    ),
    '#required' => TRUE,
  );

  foreach ($template_options['available'] as $template_name => $template_info) {
    $build['layout_paragraphs_templates']['available']['#options'][$template_name] = $template_info->title;
    $build['layout_paragraphs_templates']['available'][$template_name]['#description'] = $template_info->preview;
  }
  $build['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('form-actions')),
  );
  $build['actions']['submit'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-paragraphs-templates-select'),
    ),
    'button' => array(
      '#type' => 'submit',
      '#value' => t('Change layout template'),
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'layout_paragraphs_template_change_ajax_callback',
      ),
      '#submit' => array(
        'layout_paragraphs_select_template_change_submit',
      ),
    ),
    '#weight' => 100,
  );

  return $build;
}

/**
 * Add an interface to reset the tempstore for Layout Paragraphs layout to what
 * is saved in the database.
 */
function layout_paragraphs_reset_template_form($form, &$form_state, $layout) {
  $build = [];
  $build['#attributes'] = array(
    'class' => array('layout-paragraphs-settings-form'),
  );

  $build['intro'] = array(
    '#markup' => '<p class="description">' . t(
      'If you wish to reset the layout to the last saved state and lose all unsaved changes, click below.'
    ) . '</p>',
  );
  $build['layout_paragraphs_layout_name'] = array(
    '#type' => 'hidden',
    '#value' => $layout->name,
  );
  $build['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('form-actions')),
  );
  $build['actions']['submit'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-paragraphs-templates-select'),
    ),
    'button' => array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'layout_paragraphs_template_change_ajax_callback',
      ),
      '#submit' => array(
        'layout_paragraphs_reset_template_change_submit',
      ),
    ),
    '#weight' => 100,
  );

  return $build;
}

/**
 * Submit handler for layout_paragraphs_select_template_form().
 */
function layout_paragraphs_select_template_change_submit(&$form, &$form_state) {
  $new_template_name = $form_state['input']['available'];
  $layout_paragraphs_layout_name = $form_state['input']['layout_paragraphs_layout_name'];
  list($entity_type, $bundle, $field_name, $entity_id) = explode('.', $layout_paragraphs_layout_name);
  $layout = layout_paragraphs_get_layout_tempstore($layout_paragraphs_layout_name);
  $layout->setLayoutTemplate($new_template_name);
  layout_paragraphs_set_layout_tempstore($layout);
}

/**
 * Submit handler for layout_paragraphs_reset_template_form().
 */
function layout_paragraphs_reset_template_change_submit(&$form, &$form_state) {
  $layout_paragraphs_layout_name = $form_state['input']['layout_paragraphs_layout_name'];
  $layout = layout_paragraphs_layout_load($layout_paragraphs_layout_name, TRUE, 'database');
  layout_paragraphs_set_layout_tempstore($layout);
}

/**
 * Ajax callback for layout_paragraphs_select_template_form() to close the modal
 * and redirect back to the entity edit form.
 */
function layout_paragraphs_template_change_ajax_callback(&$form, &$form_state) {
  $layout_name = $form_state['input']['layout_paragraphs_layout_name'];
  $edit_path = tempstore_get('layout_paragraphs:edit_path', $layout_name);
  // Determine the entity edit form to reload based on the layout name.
  list($entity_type, $bundle, $field_name, $entity_id) = explode('.', $layout_name);

  switch ($form_state['triggering_element']['#value']) {
    case t('Change layout template'):
      backdrop_set_message(t('The layout template has been changed. Please review the positions of all content within the layout, and save to make the changes permanent.'));
      break;

    case t('Reset'):
      backdrop_set_message(t('The layout has been reset to the last saved state.'));
      break;
  }

  $commands = array(
    '#type' => 'ajax',
    '#commands' => array(),
  );
  $commands['#commands'][] = ajax_command_close_modal_dialog();
  $commands['#commands'][] = ajax_command_redirect($edit_path);
  return $commands;
}

/**
 * Menu callback; Show the region styles form.
 */
function layout_paragraphs_configure_region_page($form, &$form_state, Layout $layout, $renderer_name, $region_name, $paragraphs_block_key = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['layout'] = $layout;
  $form_state['region_name'] = $region_name;
  $layout_template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
  if (!array_key_exists($region_name, $layout_template_info['regions'])) {
    return MENU_NOT_FOUND;
  }

  $form['#tree'] = TRUE;

  $options = array(
    '' => 'No inner wrapper',
    'div' => 'DIV',
    'nav' => 'NAV',
    'aside' => 'ASIDE',
    'section' => 'SECTION',
  );
  $form['element'] = array(
    '#title' => t('HTML element for inner wrapper'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($layout->settings[$region_name]['element']) ? $layout->settings[$region_name]['element'] : '',
  );
  $form['classes'] = array(
    '#title' => t('CSS classes for inner wrapper'),
    '#type' => 'textfield',
    '#default_value' => isset($layout->settings[$region_name]['classes']) ? $layout->settings[$region_name]['classes'] : '',
    '#description' => t('Separate class names with spaces. Example: <code>wrapper clearfix</code>'),
    '#states' => array(
      'disabled' => array(
        ':input[name="element"]' => array('value' => ''),
      ),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array(
      'layout_paragraphs_configure_region_page_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_save_dialog',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_paragraphs_configure_region_page_submit($form, &$form_state) {
  $region_name = $form_state['region_name'];
  $form_state['layout']->settings[$region_name]['element'] = $form_state['values']['element'];
  $form_state['layout']->settings[$region_name]['classes'] = $form_state['values']['classes'];
  layout_set_layout_tempstore($form_state['layout'], 'paragraphs');
}

/**
 * Ajax callback for layout_paragraphs_select_template_form() to return the
 * updated (hidden) form value for the Layout Paragraphs template.
 *
 * @return void
 */
function layout_paragraphs_select_template_change_ajax(&$form, &$form_state) {
  return $form['layout_paragraphs']['select_template']['layout_paragraphs_template'];
}

/**
 * Returns the paragraphs context, which is available for Layout Paragraphs
 * only.
 *
 * @return EntityLayoutContext
 *   The layout context for paragraphs.
 */
function layout_paragraphs_context() {
  $paragraphs_context = layout_create_context('paragraphs', array(
    'name' => 'paragraphs',
    'label' => t('Paragraphs'),
    'locked' => TRUE,
    'usageType' => LayoutContext::USAGE_TYPE_CUSTOM,
  ));

  return $paragraphs_context;
}

/**
 * Form callback; Main form for editing a layout Paragraphs's content.
 *
 * @ingroup forms
 */
function layout_paragraphs_content_form(&$form, &$form_state, Layout &$layout, $renderer_name = 'editor') {
  $renderer = layout_create_renderer($renderer_name, $layout);
  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = $renderer;
  }

  /** @var LayoutRendererEditorParagraphs $renderer */
  $renderer = $form_state['renderer'];

  $build['#attached'] = array(
    'library' => array(
      array('system', 'backdrop.ajax'),
      array('system', 'backdrop.announce'),
    ),
    'js' => array(
      backdrop_get_path('module', 'layout') . '/js/layout.admin.js',
    ),
    'css' => array(
      backdrop_get_path('module', 'layout') . '/css/layout.admin.css',
    ),
  );

  $build['messages'] = array(
    '#markup' => $layout->locked ? '<div class="messages warning">' . layout_paragraphs_locked_message($layout) . '</div>' : '',
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $build['content'] = array('#tree' => TRUE);
  $build['content']['block'] = array('#tree' => TRUE);
  $build['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $layout_positions = $layout->positions;
  $default_region = _layout_paragraphs_get_default_region($build['content']['positions'] ?? []);
  foreach ($renderer->layout_template_info['regions'] as $region_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($layout_positions[$region_id])) {
      $layout_positions[$region_id] = array();
    }
    if (empty($build['content']['positions'][$region_id])) {
      // Use 'hidden' instead of 'value' so the JS can access it.
      $build['content']['positions'][$region_id] = array(
        '#type' => 'hidden',
        '#default_value' => implode(',', $layout_positions[$region_id]),
      );
    }
    unset($layout_positions[$region_id]);
  }
  foreach ($layout_positions as $region_id => $blocks) {
    // If there are any blocks in the layout that are not in the template,
    // add them to the default region.
    if (!isset($build['content']['positions']['paragraph-layout-default-template'])) {
      $build['content']['positions'][$default_region] = array(
        '#type' => 'hidden',
        '#default_value' => implode(',', $blocks),
      );
    }
    else {
      $build['content']['positions'][$default_region]['#default_value'] .= ',' . implode(',', $blocks);
    }
  }
  $form_state['layout'] = $layout;
  // Although layout is already set in $form_state, we need this to make it
  // available to the theme layer.
  $build['#layout'] = $layout;

  $form['layout_paragraphs']['layout'] = $build;
}

/**
 * Administrative form for Layout Paragraphs templates.
 */
function layout_paragraphs_template_listing_page($form, &$form_state) {
  $header = array(
    'layout_paragraphs_template_info' => array(
      'data' => t('Template')),
    'type' => array(
      'data' => t('Type'),
      'sort' => 'desc',
      'class' => array(RESPONSIVE_PRIORITY_LOW),
    ),
    'operations' => t('Operations'),
  );

  backdrop_set_title(t('Layout Paragraphs templates'));
  $disabled_templates = config_get('layout_paragraphs.settings', 'disabled_templates') ?? [];

  $rows = array(
    'enabled' => array(),
    'disabled' => array(),
  );
  $form_state['hidden_templates'] = array();
  $all_templates = layout_flexible_template_load_all('paragraphs');

  foreach ($all_templates as $template_name => $template) {
    // Build the default values list.
    $enabled_by_default = FALSE;
    if (!in_array($template_name, $disabled_templates)) {
      $enabled_by_default = $default[$template_name] = TRUE;
    }
    $enabled_status = $enabled_by_default ? 'enabled' : 'disabled';

    $template_info = array(
      'preview' => (!empty($template->preview)) ? $template->preview : '',
      'title' => $template->title,
      'name' => $template_name,
    );

    // Build the list of possible options, either enabled or disabled.
    $rows[$enabled_status][$template_name] = array(
      'data' => array(
        'layout_paragraphs_template_info' => theme('layout_paragraphs_template_info', array(
          'template_info' => $template_info,
        )),
        'type' => t('Paragraphs'),
        'operations' => array(
          'data' => array(
            '#type' => 'dropbutton',
            '#links' => _layout_paragraphs_settings_get_group_operations($template_info, $enabled_by_default),
          ),
        ),
      ),
      'class' => array($enabled_status),
    );
  }

  $form['templates'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows['enabled'] + $rows['disabled'],
    '#empty' => t('There are no Layout Paragraphs templates.'),
    '#attributes' => array('class' => array('layout-list')),
  );

  return $form;
}

  /**
   * Given a path, return a list of operations for a group of layouts.
   */
function _layout_paragraphs_settings_get_group_operations($template_info, $enabled_by_default) {
  $is_default = ($template_info['name'] == 'default');
  $links = array();

  if (user_access('administer paragraphs layouts')) {
    $links['configure'] = array(
      'title' => t('Configure regions'),
      'href' => 'admin/structure/paragraphs/layouts/' . $template_info['name'] . '/configure',
    );
    $links['edit'] = array(
      'title' => t('Configure name'),
      'href' => 'admin/structure/paragraphs/layouts/' . $template_info['name'] . '/edit',
    );
    $links['clone'] = array(
      'title' => t('Clone template'),
      'href' => 'admin/structure/paragraphs/layouts/' . $template_info['name'] . '/clone',
    );
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => t('Layout Paragraphs'),
          'name' => 'layout.custom_paragraphs.' . $template_info['name'],
        ),
      );
    }
    if (!$is_default) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/structure/paragraphs/layouts/' . $template_info['name'] . '/delete',
      );
      if ($enabled_by_default) {
        $links['disable'] = array(
          'title' => t('Disable'),
          'href' => 'admin/structure/paragraphs/layouts/settings/toggle/' . $template_info['name'] . '/disable',
          'query' => array('token' => backdrop_get_token('layout-paragraphs-template-' . $template_info['name'])),
        );
      }
      else {
        $links['enable'] = array(
          'title' => t('Enable'),
          'href' => 'admin/structure/paragraphs/layouts/settings/toggle/' . $template_info['name'] . '/enable',
          'query' => array('token' => backdrop_get_token('layout-paragraphs-template-' . $template_info['name'])),
        );
      }
    }

    return $links;
  }
}

/**
 * Form to add a flexible Layout Paragraphs template.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_settings_add_form() {
  $config = array(
    'is_new' => TRUE,
    'layout_template_type' => 'paragraphs',
    'is_full_page' => FALSE,
  );
  $layout_paragraphs_template = new LayoutFlexibleTemplate($config);
  backdrop_set_title(t('Add template'));
  return backdrop_get_form('layout_paragraphs_template_settings_edit_form', $layout_paragraphs_template);
}

/**
 * Helper function to determine the layout template for a given entity/instance.
 *
 * @param int $entity_id
 *   The ID of the entity.
 * @param string $entity_type
 *   The type of the entity.
 * @param string $bundle
 *   The bundle of the entity.
 * @param string $field_name
 *   The name of the field.
 * @param array $available_templates
 *   An array of template machine names that are available for this field.
 *
 * @return string $layout_template
 *   The machine name of the layout template, or 'default' if there was no
 *   database entry.
 */
function _layout_paragraphs_determine_layout_template(int $entity_id, string $entity_type, string $bundle, string $field_name, array $available_templates) {
  $allow_all = TRUE;
  foreach ($available_templates as $template_name) {
    if (!empty($template_name)) {
      // If any template is specifically checked, we will only allow those.
      $allow_all = FALSE;
    }
  }
  $result = db_select('layout_paragraphs', 'lp')
    ->fields('lp', array('layout_template'))
    ->condition('entity_id', $entity_id)
    ->condition('entity_type', $entity_type)
    ->condition('bundle', $bundle)
    ->condition('field_name', $field_name)
    ->execute()
    ->fetchAssoc();

  if (!empty($result) && ($allow_all || (!$allow_all && !empty($available_templates[$result['layout_template']])))) {
    return $result['layout_template'];
  }
  return 'default';
}

/**
 * Determine whether a layout paragraph template name already exists.
 *
 * @see layout_paragraphs_template_settings_edit_form()
 * @see form_validate_machine_name()
 */
function _layout_paragraphs_template_exists($template_name) {
  $template = layout_flexible_template_load($template_name, 'paragraphs');
  return !empty($template);
}

/**
 * Form to configure a flexible Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The loaded flexible template object.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_settings_edit_form($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template) {
  $form_state['flexible_template'] = $layout_paragraphs_template;
  $template_exists = layout_flexible_template_load($layout_paragraphs_template->name, 'paragraphs');

  $form['name'] = array(
    '#title' => t('Template name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $layout_paragraphs_template->title,
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#default_value' => $layout_paragraphs_template->name,
    '#disabled' => ($template_exists && empty($layout_paragraphs_template->is_new)),
    '#machine_name' => array(
      'exists' => '_layout_paragraphs_template_exists',
    ),
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => $layout_paragraphs_template->description,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#submit' => array(
      'layout_paragraphs_template_settings_form_submit',
    ),
    '#value' => t('Save and configure'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#limit_validation_errors' => array(array('name', 'machine_name')),
    '#submit' => array(
      'layout_paragraphs_template_settings_form_cancel',
    ),
  );

  return $form;
}

/**
 * Submit handler that cancels the flexible Layout Paragraphs template form.
 *
 * @see layout_paragraphs_template_settings_edit_form()
 */
function layout_paragraphs_template_settings_form_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts';
}

/**
 * Submit handler that saves a flexible Layout Paragraphs template.
 *
 * @see layout_paragraphs_template_settings_edit_form()
 */
function layout_paragraphs_template_settings_form_submit($form, &$form_state) {
  /** @var LayoutFlexibleTemplate $layout_paragraphs_template */
  $layout_paragraphs_template = $form_state['flexible_template'];
  $layout_paragraphs_template->name = $form_state['values']['machine_name'];
  $layout_paragraphs_template->title = $form_state['values']['name'];
  $layout_paragraphs_template->description = $form_state['values']['description'];
  $layout_paragraphs_template->layout_template_type = 'custom_paragraphs';
  $layout_paragraphs_template->is_full_page = FALSE;
  unset($layout_paragraphs_template->is_new);
  $layout_paragraphs_template->preview = _layout_paragraphs_template_generate_svg($layout_paragraphs_template);

  $layout_paragraphs_template->save();

  layout_paragraphs_template_tempstore_set($layout_paragraphs_template);
  layout_paragraphs_reset_caches();
  backdrop_set_message(t('Layout paragraphs template "@title" saved.', array(
    '@title' => $layout_paragraphs_template->title,
  )));
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['values']['machine_name'] . '/configure';
}

/**
 * Form to manage adding/removing rows to a flexible Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The template configuration being edited.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_configure_form($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.admin');

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('This page allows you to add or remove rows, and change the order of rows by dragging them up and down. You can also change the number of regions within each row, and adjust the width of each region.'),
  );

  $messages = array();
  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => $messages,
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $form_state['flexible_template'] = &$layout_paragraphs_template;
  $layout_paragraphs_template_name = $layout_paragraphs_template->name;
  $form_state['flexible_template_name'] = $layout_paragraphs_template_name;
  $form_state['flexible_template_type'] = 'paragraphs';

  $config = array(
    'is_new' => TRUE,
    'layout_template' => $layout_paragraphs_template_name,
    'layout_template_type' => 'paragraphs',
    'is_full_page' => FALSE,
  );
  $layout = new Layout($config);

  backdrop_set_title($layout_paragraphs_template->title);

  if (empty($form_state['flexible_template'])) {
    return array();
  }

  _layout_include_files();

  $renderer_info = array(
    'class' => 'LayoutRendererParagraphs',
    'module' => 'layout_paragraphs',
    'name' => 'paragraphs',
    'path' => backdrop_get_path('module', 'layout_paragraphs') . '/plugins/renderers',
  );
  // $renderer = new LayoutRendererParagraphs($layout, $renderer_info);
  $renderer = layout_create_renderer('paragraphs', $layout);
  $form['content'] = array(
    '#type' => 'item',
    '#id' => 'layout-flexible-content',
  );
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $form['content']['row_positions'] = array(
    // Use 'hidden' instead of 'value' so the JS can access it.
    '#type' => 'hidden',
    '#default_value' => implode(',', array_keys($layout_paragraphs_template->rows)),
  );

  $form['content']['add_row'] = array(
    '#type' => 'submit',
    '#value' => t('Add row'),
    '#attributes' => array(
      'class' => array(
        'layout-flexible-add-row',
        'button-secondary',
      ),
    ),
    '#submit' => array(
      'layout_paragraphs_template_add_row',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save layout template'),
    '#submit' => array(
      'layout_paragraphs_template_configure_form_submit',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array(
      'layout_paragraphs_template_configure_form_cancel',
    ),
  );

  return $form;
}

/**
 * Form to manage cloning a Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The template configuration being edited.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_clone_form($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template) {
  backdrop_set_title(t('Clone template'));
  $new_name = $layout_paragraphs_template->name . '_clone';
  $new_title = $layout_paragraphs_template->title . ' ' . t('(Clone)');
  $all_templates = layout_flexible_template_load_all('paragraphs');
  $i = 1;
  $max = count($all_templates) + 1;
  for (; $i < $max; $i++) {
    if (isset($all_templates[$new_name])) {
      $new_name = $layout_paragraphs_template->name . '_clone_' . $i;
      $new_title = $layout_paragraphs_template->title . ' ' . t('(Clone @number)', array('@number' => $i));
    }
    else {
      break;
    }
  }
  $layout_paragraphs_template->name = $new_name;
  $layout_paragraphs_template->title = $new_title;
  $layout_paragraphs_template->is_new = TRUE;
  $build = backdrop_get_form('layout_paragraphs_template_settings_edit_form', $layout_paragraphs_template);
  $build['help'] = array(
    '#type' => 'help',
    '#markup' => t('You are cloning a Paragraphs layout template. This can be helpful to ensure matching region names, so switching between templates will flow the content into the expected regions.'),
    '#weight' => -10,
  );
  $form_state['flexible_template_source'] = $layout_paragraphs_template;
  $build['actions']['submit']['#submit'] = array(
    'layout_paragraphs_template_clone_form_submit',
  );
  return $build;
}

/**
 * Submit handler for managing cloning a Layout Paragraphs template.
 *
 * @ingroup forms
 *
 * @see layout_paragraphs_template_clone_form()
 */
function layout_paragraphs_template_clone_form_submit($form, &$form_state) {
  $template = $form_state['flexible_template_source'];
  $template->name = check_plain($form_state['input']['machine_name']);
  $template->title = check_plain($form_state['input']['name']);
  $template->description = check_plain($form_state['input']['description']);
  unset($template->is_new);
  $template->save();
  layout_paragraphs_reset_caches();
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $template->name . '/configure';
}

/**
 * Handler for layout_paragraphs_template_configure_form() to add a new row.
 */
function layout_paragraphs_template_add_row($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/row/add/select-widths';
}

/**
 * Submit handler that cancels in-progress changes.
 *
 * @see layout_paragraphs_template_configure_form()
 */
function layout_paragraphs_template_configure_form_cancel($form, &$form_state) {
  tempstore_clear('layout.paragraphs', $form_state['flexible_template_name']);
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts';
}

/**
 * Submit handler that saves the configured flexible Layout Paragraphs template.
 *
 * @see layout_paragraphs_template_configure_form()
 */
function layout_paragraphs_template_configure_form_submit($form, &$form_state) {
  // Load from tempstore rather than using $form_state, in the event a row
  // has been removed in an AJAX request.
  $layout_paragraphs_template = layout_paragraphs_tempstore_load($form_state['flexible_template_name']);
  $positions = array();

  if (!empty($form_state['values']['row_positions'])) {
    $rows = array_filter(explode(',', $form_state['values']['row_positions']));
    foreach ($rows as $position) {
      if (isset($layout_paragraphs_template->rows[$position])) {
        $positions[$position] = $layout_paragraphs_template->rows[$position];
      }
    }
  }

  $layout_paragraphs_template->rows = $positions;
  $layout_paragraphs_template->preview = _layout_paragraphs_template_generate_svg($layout_paragraphs_template);
  $layout_paragraphs_template->save();

  tempstore_clear('layout.paragraphs', $form_state['flexible_template_name']);
  layout_paragraphs_reset_caches();
  backdrop_set_message(t('Layout Paragraphs template "@title" saved.', array(
    '@title' => $layout_paragraphs_template->title,
  )));
}

/**
 * Form to select the region widths for a flexible Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_region_style_select($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template, $original_row) {
  $new_row = ($original_row == 'add');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.admin');
  $form_state['flexible_template_name'] = $layout_paragraphs_template->name;
  $form_state['original_row'] = $original_row;

  $row_styles = layout_flexible_row_styles();
  $count_options = array();
  foreach ($row_styles as $name => $row_style) {
    $count_options[$row_style['region_count']] = $row_style['region_count'];
  }

  $row_style_in_use = 'region_12';
  $default_region_count = 1;
  if (!$new_row) {
    if (isset($form_state['values']['region_style'])) {
      $row_style_in_use = $form_state['values']['region_style'];
    }
    else {
      $row_style_in_use = $layout_paragraphs_template->rows[$original_row]['contains'];
    }
    $default_region_count = $row_styles[$row_style_in_use]['region_count'];
  }
  $form['region_count'] = array(
    '#title' => t('Number of regions'),
    '#type' => 'radios',
    '#default_value' => $default_region_count,
    '#parents' => array('region_count'),
    '#options' => $count_options,
    '#ajax' => array(
      'callback' => 'layout_paragraphs_template_edit_region_style_ajax',
      'wrapper' => 'row_settings',
      'trigger_as' => array('name' => 'region_count_update'),
    ),
  );
  $form['region_count_update'] = array(
    '#type' => 'submit',
    '#value' => t('Update regions'),
    '#attributes' => array('class' => array('js-hide')),
    '#name' => 'region_count_update',
    '#validate' => array(),
    '#submit' => array('layout_paragraphs_template_edit_region_style_submit'),
    '#ajax' => array(
      'callback' => 'layout_paragraphs_template_edit_region_style_ajax',
      'wrapper' => 'row_settings',
    ),
  );

  $form['row_settings'] = array(
    '#type' => 'container',
    '#id' => 'row_settings',
    '#parents' => array('row_settings'),
  );

  $region_count = isset($form_state['values']['region_count']) ? $form_state['values']['region_count'] : $default_region_count;

  $options = array();
  foreach ($row_styles as $name => $row_style) {
    if ($row_style['region_count'] == $region_count) {
      $options[$name] = theme('layout_flexible_template_style_option', array(
        'row_style' => $row_style,
      ));
    }
  }

  if (empty($options)) {
    $options['region_12'] = '12:0';
  }

  $default_region_style = $row_style_in_use;
  if (!isset($options[$row_style_in_use])) {
    $default_region_style = key($options);
  }
  $form['row_settings']['region_style'] = array(
    '#title' => t('Region widths'),
    '#type' => 'radios',
    '#default_value' => $default_region_style,
    '#options' => $options,
  );

  $form['submit_region_style'] = array(
    '#type' => 'submit',
    '#value' => t('Choose region widths'),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
    '#submit' => array(
      'layout_paragraphs_template_region_count_load',
    ),
  );

  $submit_button_value = $new_row ? t('Continue') : t('Save region widths');
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#name' => 'submit',
    '#value' => $submit_button_value,
    '#attributes' => array('class' => array('layout-title-button')),
    '#submit' => array(
      'layout_paragraphs_template_region_style_select_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );
  if (isset($layout_paragraphs_template->rows[$original_row]['in_progress'])) {
    $cancel_submit_callback = 'layout_paragraphs_template_region_style_select_cancel';
    $cancel_ajax_callback = 'layout_ajax_form_open_dialog';
  }
  else {
    $cancel_submit_callback = 'layout_paragraphs_template_cancel';
    $cancel_ajax_callback = 'layout_paragraphs_template_cancel_ajax';
  }
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#submit' => array(
      $cancel_submit_callback,
    ),
    '#ajax' => array(
      'callback' => $cancel_ajax_callback,
    ),
  );

  return $form;
}

/**
 * Submit handler that cancels the region style selection form.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/configure';
}

/**
 * AJAX handler to cancels the region style selection form.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_cancel_ajax($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Submit handler that cancels the region style selection form.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_region_style_select_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/configure';
}

/**
 * Submit handler that saves the region style selection form.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_region_style_select_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/configure/' . $form_state['values']['region_style'];
}

/**
 * Form to add or edit a row on a flexible Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 * @param string $region_style
 *   The selected region style.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_edit_row_form($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template, $original_row, $region_style = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout_paragraphs', 'layout_paragraphs.admin');
  $form_state['flexible_template'] = &$layout_paragraphs_template;
  $layout_paragraphs_template_name = $layout_paragraphs_template->name;

  $form_state['flexible_template_name'] = $layout_paragraphs_template_name;
  $form_state['original_row'] = $original_row;

  $row_data = array();
  if ($original_row != 'add') {
    if (isset($layout_paragraphs_template->rows[$original_row]['in_progress'])) {
      $row_data = $layout_paragraphs_template->rows[$original_row]['in_progress'];
    }
    else {
      $row_data = $layout_paragraphs_template->rows[$original_row];
    }
  }

  $region_style = $region_style ? $region_style : $row_data['contains'];
  $form_state['region_style'] = $region_style;

  $row_styles = layout_flexible_row_styles();
  $selected_style = $row_styles[$region_style]['name'];
  $region_count = $row_styles[$region_style]['region_count'];

  if ($original_row == 'add') {
    backdrop_set_title(t('Add new row'));
  }
  else {
    backdrop_set_title(t('Configure row !original_row', array('!original_row' => $original_row)));
  }

  $form['region_style'] = array(
    '#type' => 'item',
    '#title' => t('Selected region widths'),
    '#markup' => $selected_style,
  );

  if ($original_row != 'add') {
    $form['change_region_style'] = array(
      '#type' => 'submit',
      '#value' => t('Change region widths'),
      '#attributes' => array('class' => array('layout-link-button')),
      '#submit' => array(
        'layout_paragraphs_template_change_region_style',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
    );
  }

  $form['region_names'] = array(
    '#type' => 'fieldset',
    '#title' => t('Regions'),
    '#collapsed' => $original_row != 'add',
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  $form_state['last_region_number'] = layout_paragraphs_template_get_last_region_number($layout_paragraphs_template);
  for ($i = 0; $i < $region_count; $i++) {
    $form['region_names']['region_' . $i] = array(
      '#type' => 'container',
    );
    if (isset($row_data['region_names']['region_' . $i]['label'])) {
      $region_name = $row_data['region_names']['region_' . $i]['label'];
    }
    $form['region_names']['region_' . $i]['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Region name') . ' ' . ($i + 1),
      '#default_value' => !empty($region_name) ? $region_name : t('Region') . ' ' . ($form_state['last_region_number'] + $i + 1),
    );
    $form['region_names']['region_' . $i]['name'] = array(
      '#type' => 'machine_name',
      '#description' => t('A machine name to be used as the default CSS class. It must only contain uppercase letters, lowercase letters, numbers, hyphens and underscores.'),
      '#default_value' => !empty($row_data['region_names']['region_' . $i]['name']) ? $row_data['region_names']['region_' . $i]['name'] : '',
      '#maxlength' => 255,
      '#field_prefix' => '<span dir="ltr">l-region--',
      '#field_suffix' => '</span>&lrm;',
      '#size' => 15,
      '#disabled' => FALSE,
      '#machine_name' => array(
        'exists' => '_layout_paragraphs_template_region_name_exists',
        'source' => array('region_names', 'region_' . $i, 'label'),
        'standalone' => TRUE,
        'label' => t('Name'),
        'replace_pattern' => '[^a-z0-9-]+',
        'replace' => '-',
      ),
    );
    if (isset($row_data['region_names']['region_' . $i]['region_class_enable'])) {
      $region_class_enable = $row_data['region_names']['region_' . $i]['region_class_enable'];
    }
    $form['region_names']['region_' . $i]['region_class_enable'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add CSS classes'),
      '#default_value' => !empty($region_class_enable) ? $region_class_enable : 0,
    );
    if (isset($row_data['region_names']['region_' . $i]['classes'])) {
      $classes = $row_data['region_names']['region_' . $i]['classes'];
    }
    $form['region_names']['region_' . $i]['classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Additional region CSS classes'),
      '#description' => t('Separate multiple classes with spaces.'),
      '#default_value' => !empty($classes) ? $classes : '',
      '#states' => array(
        'visible' => array(
          ':input[name="region_names[region_' . $i . '][region_class_enable]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $form['region_styles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Row Style'),
    '#collapsed' => $original_row != 'add',
    '#collapsible' => TRUE,
  );

  $options = array(
    'div' => 'DIV',
    'nav' => 'NAV',
    'aside' => 'ASIDE',
    'section' => 'SECTION',
    'header' => 'HEADER',
    'footer' => 'FOOTER',
    'main' => 'MAIN',
  );
  $form['region_styles']['element'] = array(
    '#title' => t('Row wrapper tag'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($row_data['element']) ? $row_data['element'] : '',
  );

  $form['region_styles']['container'] = array(
    '#title' => t('Row width behavior'),
    '#type' => 'radios',
    '#options' => array(
      'container' => t('Fixed maximum width'),
      'container_fluid' => t('Fluid width'),
      'no_container' => t('Full width'),
    ),
    '#default_value' => isset($row_data['container']) ? $row_data['container'] : 'container_fluid',
  );
  $form['region_styles']['container']['container']['#description'] = t('Adds the <code>container</code> class to the row.');
  $form['region_styles']['container']['container_fluid']['#description'] = t('Adds the <code>container-fluid</code> class to the row (no <code>max-width</code>, but <code>padding</code>).');
  $form['region_styles']['container']['no_container']['#description'] = t('No container-related classes added to the row (e.g. for Hero blocks).');

  $form['region_styles']['row_classes'] = array(
    '#title' => t('Additional row CSS classes'),
    '#type' => 'textfield',
    '#default_value' => isset($row_data['classes']) ? $row_data['classes'] : '',
    '#description' => t('Separate multiple classes with spaces.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#name' => 'submit',
    '#value' => t('Save configuration'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#validate' => array(
      'layout_paragraphs_template_edit_row_validate',
    ),
    '#submit' => array(
      'layout_paragraphs_template_edit_row_form_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_paragraphs_template_edit_row_ajax',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#submit' => array(
      'layout_paragraphs_template_cancel',
    ),
    '#ajax' => array(
      'callback' => 'layout_paragraphs_template_cancel_ajax',
    ),
  );

  return $form;
}

/**
 * Callback: Check if machine_name is unique.
 *
 * Even though callback is required for the Form API machine_name field type,
 * the region classes are not required to be unique. It will accept any
 * machine_name.
 *
 * @param string $machine_name
 *   The machine name for a region.
 *
 * @return bool
 *   Whether or not the region name is taken.
 *
 * @see layout_paragraphs_template_edit_row_form()
 * @see form_validate_machine_name()
 */
function _layout_paragraphs_template_region_name_exists(string $machine_name, $element, $form_state) {
  $layout_paragraphs_template = layout_paragraphs_tempstore_load($form_state['flexible_template_name']);
  foreach ($layout_paragraphs_template->rows as $row) {
    foreach ($row['region_names'] as $region_name) {
      if ($region_name['name'] == $machine_name) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
 * Returns the last number of all regions named "Region X" where X is a number.
 */
function layout_paragraphs_template_get_last_region_number(LayoutFlexibleTemplate $layout_paragraphs_template, $form_state = NULL) {
  $last = 0;
  $region_names = array();
  if ($form_state) {
    $regions = array_filter($form_state['values']['region_names']);
    $last = $form_state['last_region_number'];
    foreach ($regions as $key => $region) {
      $region_names[] = $region['label'];
    }
  }
  else {
    foreach ($layout_paragraphs_template->rows as $row) {
      foreach ($row['region_names'] as $key => $region) {
        $region_names[] = $region['label'];
      }
    }
  }

  foreach ($region_names as $region) {
    $str = explode('Region ', $region);
    if (!empty($str[1]) && is_numeric($str[1]) && $str[1] > $last) {
      $last = $str[1];
    }
  }

  return $last;
}

/**
 * Submit handler for the change region style button on the
 * layout_paragraphs_template_edit_row_form().
 */
function layout_paragraphs_template_change_region_style($form, &$form_state) {
  $old_row = array(
    'contains' => $form_state['region_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'region_names' => $form_state['values']['region_names'],
  );
  $form_state['flexible_template']->rows[$form_state['original_row']]['in_progress'] = $old_row;

  layout_paragraphs_template_tempstore_set($form_state['flexible_template']);

  $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/select-widths';
}

/**
 * Submit handler on the hidden button to change region widths on
 * layout_paragraphs_template_edit_row_form() if AJAX is disabled.
 */
function layout_paragraphs_template_region_count_load(&$form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Validation handler for layout_paragraphs_template_edit_row_form().
 */
function layout_paragraphs_template_edit_row_validate($form, &$form_state) {
  $last_region_number = layout_paragraphs_template_get_last_region_number($form_state['flexible_template'], $form_state);

  foreach ($form_state['values']['region_names'] as $key => $region) {
    if (empty($region)) {
      $number = str_replace('region_', '', $key);
      $form_state['values']['region_names'][$key]['label'] = t('Region') . ' ' . ($last_region_number + $number + 1);
      $last_region_number++;
    }
  }
}

/**
 * Submit handler for layout_paragraphs_template_edit_row_form().
 */
function layout_paragraphs_template_edit_row_form_submit($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  /** @var LayoutFlexibleTemplate $layout_paragraphs_template */
  $layout_paragraphs_template = $form_state['flexible_template'];
  $original_row = $form_state['original_row'];

  $new_row = array(
    'contains' => $form_state['region_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'region_names' => $form_state['values']['region_names'],
  );

  if ($original_row == 'add') {
    $uuid = new Uuid();
    $original_row = $uuid->generate();
  }

  $layout_paragraphs_template->rows[$original_row] = $new_row;

  $form_state['flexible_template'] = $layout_paragraphs_template;

  $form_state['ajax_rebuild_form'] = 'layout_paragraphs_template_configure_form';
  $form_state['ajax_rebuild_args'] = array($layout_paragraphs_template);
  $form_state['ajax_update'] = array('content');
  layout_paragraphs_template_tempstore_set($layout_paragraphs_template);
  layout_paragraphs_reset_caches();

  if (!backdrop_is_ajax()) {
    $form_state['redirect'] = 'admin/structure/paragraphs/layouts/' . $form_state['flexible_template_name'] . '/configure';
  }
}

/**
 * AJAX callback to change the region count.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_edit_region_style_ajax($form, &$form_state) {
  tempstore_clear('layout.paragraphs', $form_state['flexible_template_name']);
  return $form['row_settings'];
}

/**
 * Submit callback to change the region count.
 *
 * @see layout_paragraphs_template_region_style_select()
 */
function layout_paragraphs_template_edit_region_style_submit($form, &$form_state) {
  // Remove the current region style when switching between region counts.
  $form_state['input']['region_style'] = NULL;
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX callback on the submit button for editing a layout row.
 *
 * @see layout_paragraphs_template_edit_row_form()
 */
function layout_paragraphs_template_edit_row_ajax($form, $form_state) {
  $commands = array();

  // Delete cache to ensure the new row is displayed.
  layout_paragraphs_reset_caches();

  // Display error messages in the form if any.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array(
      'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
    ));
  }
  else {
    $commands[] = ajax_command_close_modal_dialog();

    $update_ajax = layout_ajax_form_update($form, $form_state);
    $commands = array_merge($commands, $update_ajax['#commands']);
  }

  // Update the messages area.
  $commands[] = ajax_command_remove('#messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Menu callback to delete a row from a flexible Layout Paragraphs template.
 *
 * @param LayoutFlexibleTemplate $layout_paragraphs_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_delete_row(LayoutFlexibleTemplate $layout_paragraphs_template, $original_row) {
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'layout-region-' . $original_row)) {
    return MENU_ACCESS_DENIED;
  }

  $commands = array();
  $layout_paragraphs_template_name = $layout_paragraphs_template->name;

  unset($layout_paragraphs_template->rows[$original_row]);
  layout_paragraphs_template_tempstore_set($layout_paragraphs_template);
  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#flexible-row--' . $original_row);

    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Row "@title" removed.', array(
      '@title' => $layout_paragraphs_template->title,
    )));
    backdrop_goto('admin/structure/paragraphs/layouts/' . $layout_paragraphs_template_name . '/configure');
    return array();
  }
}

/**
 * Menu callback; Delete a flexible template.
 *
 * @ingroup forms
 */
function layout_paragraphs_template_delete_form($form, &$form_state, LayoutFlexibleTemplate $layout_paragraphs_template) {
  $form_state['flexible_template'] = &$layout_paragraphs_template;
  $question = t('Delete flexible template @title?', array(
    '@title' => $layout_paragraphs_template->title,
  ));
  $description = t('This action cannot be undone.');
  backdrop_set_title($question, PASS_THROUGH);

  $all_templates = layout_flexible_template_load_all('paragraphs');
  unset($all_templates[$layout_paragraphs_template->name]);
  $replacement_templates = array();
  foreach ($all_templates as $template) {
    $replacement_templates[$template->name] = $template->title;
  }
  // Check if this template is in use.
  $in_use = _layout_paragraphs_template_in_use($layout_paragraphs_template->name);

  if (!empty($in_use)) {
    $form['replacement'] = array(
      '#title' => t('Replacement layout template'),
      '#type' => 'select',
      '#options' => $replacement_templates,
      '#required' => TRUE,
    );
    $description = t('This template is currently in use, so you must select a replacement layout template. If regions in the two templates do not match, all Paragraphs will be moved to a default region in the new template, which may not be ideal. Please test first and be sure you wish to remove this template.') . ' ' . $description;
  }
  else {
    $form['replacement'] = array(
      '#type' => 'hidden',
      '#value' => FALSE,
    );
  }

  $form['description'] = array(
    '#markup' => $description,
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#disabled' => !empty($errors),
    '#value' => t('Delete template'),
    '#attributes' => array('class' => array('button-danger')),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/paragraphs/layouts',
  );

  $form['#attributes']['class'][] = 'confirmation';
  $form['#theme'] = 'confirm_form';

  return $form;
}

/**
 * Submit handler that deletes a flexible Layout Paragraphs template.
 *
 * @see layout_paragraphs_template_delete_form()
 */
function layout_paragraphs_template_delete_form_submit(&$form, &$form_state) {
  /** @var LayoutFlexibleTemplate $layout_paragraphs_template */
  $layout_paragraphs_template = &$form_state['flexible_template'];
  $replacement_template_name = check_plain($form_state['values']['replacement']);
  if (!empty($replacement_template_name)) {
    // Update all layouts using this template to use the replacement template.
    db_update('layout_paragraphs')
      ->fields(['layout_template' => $replacement_template_name])
      ->condition('layout_template', $layout_paragraphs_template->name)
      ->execute();

    // Update field instances that allow the old template to use the new one.
    $field_instances_all = field_info_instances();
    $field_changes = FALSE;
    foreach ($field_instances_all as $field_instance_entity_type) {
      foreach ($field_instance_entity_type as $field_instance_bundle) {
        foreach ($field_instance_bundle as $field_instance) {
          if ($field_instance['widget']['type'] == 'layout_paragraphs' &&
              in_array($layout_paragraphs_template->name, $field_instance['widget']['settings']['available_layout_templates'])) {
                $config = config('field.instance.' . $field_instance['entity_type'] . '.' . $field_instance['bundle'] . '.' . $field_instance['field_name']);
                $available_layout_templates = $config->get('widget.settings.available_layout_templates');
                $available_layout_templates = array_diff(
                  $field_instance['widget']['settings']['available_layout_templates'],
                  [$layout_paragraphs_template->name]
                );
                $available_layout_templates[$replacement_template_name] = $replacement_template_name;
                $config->set('widget.settings.available_layout_templates', $available_layout_templates);
                $config->save();
                $field_changes = TRUE;
          }
        }
      }
    }
    if ($field_changes) {
      field_cache_clear();
    }
  }

  $layout_paragraphs_template->delete();
  layout_paragraphs_reset_caches();

  $message = t('The layout template "@title" has been deleted.', array(
    '@title' => $layout_paragraphs_template->title,
  ));
  if (!empty($replacement_template_name)) {
    $message .= ' ' . t('All layouts using this template have been updated to use the "@title" template instead.', array('@title' => $replacement_template_name));
  }
  backdrop_set_message($message);
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts';
}

/**
 * Helper to check if a layout template is in use.
 *
 * @param string $layout_template_name
 *   The layout template machine name.
 *
 * @return array
 *   An array of lp_id values where the layout template is in use, or an empty
 *   array if not in use.
 */
function _layout_paragraphs_template_in_use(string $layout_template_name) {
  $in_use = [];
  $layouts = db_query("SELECT id FROM {layout_paragraphs} WHERE layout_template = :layout_template", [
    ':layout_template' => $layout_template_name,
  ])->fetchAll();
  foreach ($layouts as $layout) {
    $in_use[] = $layout->id;
  }
  return $in_use;
}

/**
 * Generate an SVG image to be used as a Paragraphs layout template thumbnail.
 * @todo If this goes in core it may be possible to use that and remove this.
 * @see https://github.com/backdrop/backdrop-issues/issues/4104
 *
 * @param object $template
 *   A layout template object.
 *
 * @return string $svg
 *   The SVG of the generated thumbnail image.
 */
function _layout_paragraphs_template_generate_svg(object $template) {
  $template_rows = $template->rows;
  $flexi_styles = layout_flexible_row_styles();
  $thumbnail_rows = array();
  $row_color = 'currentColor';
  $svg = "<div class='layout-icon'><svg width='100%' height='100%' viewBox='0 0 100 100' preserveAspectRatio='none'>";
  $thumb_width = 100;
  $thumb_height = 100;
  $separator_size = 3;
  foreach ($template_rows as $row) {
    $thumbnail_rows[] = $flexi_styles[$row['contains']]['split'];
  }
  $rows_count = count($thumbnail_rows);
  $horizontal_separator_count = $rows_count - 1;
  $available_vertical_space = $thumb_height - ($horizontal_separator_count * $separator_size);
  // Divide the vertical space among the number of rows.
  $row_height = ($rows_count) ? $available_vertical_space / ($rows_count) : $available_vertical_space;

  $starting_y = 0;
  foreach ($thumbnail_rows as $index => $region_split) {
    $splits = explode(':', $region_split);
    $regions_count = count(array_filter($splits));
    $vertical_separator_count = $regions_count - 1;
    $available_horizontal_space = $thumb_width - ($vertical_separator_count * $separator_size);
    $current_row_height = $row_height;
    $starting_x = 0;
    foreach ($splits as $split) {
      if (!empty($split) && $starting_x < $thumb_width) {
        $split_width = ($split * $available_horizontal_space) / 100;
        $svg .= "<rect x='$starting_x%' y='$starting_y%' width='" . $split_width . "%' height='" . $current_row_height . "%' fill='$row_color' />";
        $starting_x += $split_width + $separator_size;
      }
    }

    $starting_y += $current_row_height + $separator_size;
  }

  $svg .= "</svg></div>";
  return $svg;
}

/**
 * Return block info for Paragraphs types when editing with Layout Paragraphs.
 */
function layout_paragraphs_get_block_info($delta = NULL) {
  $blocks = array();
  $bundles = paragraphs_bundle_load();
  foreach ($bundles as $machine_name => $bundle) {
    $blocks[$machine_name] = array(
      'info' => $bundle->name,
      'description' => $bundle->description,
      'required contexts' => ['paragraphs'],
    );
  }
  return array(
    'layout_paragraphs' => $blocks,
  );
}

/**
 * Title callback for the Paragraphs block add page.
 */
function layout_paragraphs_block_add_title($layout, $renderer_name, $region_name) {
  $title = t('Add Paragraph');
  $field_instance = _layout_paragraphs_get_field_instance($layout->name);
  if (!empty($field_instance)) {
    $title = t('Add @paragraph_title', array(
      '@paragraph_title' => $field_instance['settings']['title'],
    ));
  }
  return $title;
}

/**
 * Menu callback for adding a Paragraph block into a layout. Displays a list of
 * new Paragraph blocks to be added to a layout.
 */
function layout_paragraphs_block_add_page(Layout $layout, $renderer_name, $region_name, $paragraphs_block_key = NULL) {
  // Ensure the latest version of the layout is loaded, e.g. in case of an
  // Ajax template change.
  $tempstore_layout = layout_paragraphs_tempstore_load($layout->name);
  if ($tempstore_layout) {
    $layout = $tempstore_layout;
  }
  /** @var Layout $layout */
  $layout->setContexts('paragraphs', layout_paragraphs_context());
  $form_state['layout'] = $layout;
  $template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
  if (!array_key_exists($region_name, $template_info['regions'])) {
    return MENU_NOT_FOUND;
  }

  // If a Paragraphs block has been selected, return the block configuration
  // form.
  if (isset($paragraphs_block_key)) {
    list($module, $delta) = explode(':', $paragraphs_block_key);
    $block_info = layout_paragraphs_get_block_info($delta);
    if (!$block_info) {
      return MENU_NOT_FOUND;
    }

    $block_data = [
      'paragraphs_item_id' => NULL,
      'layout_name' => $layout->name,
    ];
    $block = new BlockParagraphs($paragraphs_block_key, $block_data);
    return backdrop_get_form('layout_paragraphs_block_configure_form', $layout, $block, $renderer_name, $region_name);
  }

  $module = 'layout_paragraphs';
  $blocks = layout_paragraphs_get_block_info();

  // Assemble all blocks into a single list and sort.
  $block_list = array(
    'selected' => array(),
    'deselected' => array(),
  );
  $field_instance = _layout_paragraphs_get_field_instance($layout->name);
  foreach ($blocks as $module => $module_blocks) {
    if (empty($module_blocks)) {
      continue;
    }
    foreach ($module_blocks as $delta => $block_info) {
      $delta_key = (empty($field_instance['settings']['allowed_bundles'][$delta]) || $field_instance['settings']['allowed_bundles'][$delta] == -1) ? 'deselected' : 'selected';
      $required_contexts = isset($block_info['required contexts']) ? $block_info['required contexts'] : array();
      // Skip blocks that don't have context requirements met.
      if ($layout->hasContexts($required_contexts)) {
        $block_list[$delta_key][$module . ':' . $delta] = $block_info;
      }
    }
  }
  // If none are selected, all are allowed.
  $block_list = (empty($block_list['selected'])) ? $block_list['deselected'] : $block_list['selected'];
  backdrop_sort($block_list, array('info' => SORT_STRING));

  $output['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Search'),
    '#attributes' => array(
      'id' => array('layout-block-list-search'),
    ),
    // Hide the search if no JavaScript is available.
    '#wrapper_attributes' => array(
      'class' => array('js-show'),
    ),
  );
  $output['block_list'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-block-list'),
    ),
  );

  $output['block_list']['block_list_inner'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-block-list-wrapper'),
    ),
  );

  foreach ($block_list as $key => $block_info) {
    $output['block_list']['block_list_inner'][$key] = array(
      '#type' => 'item',
      '#markup' => l(
        $block_info['info'],
        'admin/structure/paragraphs/layouts/manage/' . $layout->name . '/add-block/' . $renderer_name . '/' . $region_name . '/' . $key,
        array(
          'attributes' => array(
            'class' => array('use-ajax block-item'),
            'data-dialog' => TRUE,
            'data-dialog-options' => json_encode(array(
              'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
            )),
          ),
        )
      ),
      '#description' => isset($block_info['description']) ? filter_xss($block_info['description']) : NULL,
      '#wrapper_attributes' => array('class' => array('layout-block-add-row')),
    );
  }

  return $output;
}

/**
 * Form callback; Add or edit a Paragraphs block within a Paragraphs layout.
 *
 * @ingroup forms
 */
function layout_paragraphs_block_configure_form($form, &$form_state, Layout $layout, Block $block, $renderer_name, $region_name = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout_paragraphs.admin');
  $paragraph_name = $block->getAdminTitle();

  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = layout_create_renderer($renderer_name, $layout);
  }
  if (!isset($region_name)) {
    $region_name = $layout->getBlockPosition($block->uuid);
  }

  if (empty($block->uuid)) {
    $form['#title'] = t('Add @title', array('@title' => $paragraph_name));
  }
  else {
    $form['#title'] = t('Configure @title', array('@title' => $paragraph_name));
  }

  $template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
  $form['region'] = array(
    '#type' => 'select',
    '#title' => t('Region'),
    '#default_value' => $region_name,
    '#options' => $template_info['regions'],
    '#access' => backdrop_is_html(), // Only show this on non-JS requests.
  );

  // We overload the layout object with an "in_progress" key to hold information
  // about the block currently being edited, this eases the multi-dialog
  // workflow for a sequence of dialogs affecting existing or new blocks.
  if (!isset($layout->in_progress['block']) || $layout->in_progress['block']->uuid !== $block->uuid) {
    $layout->in_progress = array(
      'block' => $block,
      'region_name' => $region_name,
      'renderer_name' => $renderer_name,
      'section' => NULL,
    );
  }
  // If resuming an in-progress edit, pull out the region name.
  else {
    $form['region']['#default_value'] = $layout->in_progress['region_name'];
  }

  $form['actions']['#weight'] = 999;
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($block->uuid) ? t('Add @title', array(
      '@title' => $paragraph_name,
    )) : t('Update @title', array(
      '@title' => $paragraph_name,
    )),
  );

  $form['#tree'] = TRUE;
  $block->form($form, $form_state);

  // Note that we specifically use backdrop_is_ajax() in addition to
  // backdrop_is_dialog() to ensure the #ajax callback is specified when the
  // style dropdown rebuilds the form on changes.
  if (backdrop_is_dialog() || backdrop_is_ajax()) {
    $form['actions']['submit']['#ajax'] = array(
      'callback' => 'layout_paragraphs_block_configure_ajax_update',
    );
  }

  return $form;
}

/**
 * Form validate handler for layout_paragraphs_block_configure_form().
 */
function layout_paragraphs_block_configure_form_validate($form, &$form_state) {
  /** @var BlockParagraphs $block */
  $block = $form_state['block'];

  // Validate the block settings.
  $block->formValidate($form, $form_state);

  // Replace the current style handler with a new one, in the event it changed.
  $style = layout_create_handler('layout_style', $form_state['values']['style'] ?? 'LayoutStyleParagraphs');
  $style->formValidate($form, $form_state);
  $form_state['style'] = $style;
}

/**
 * Form submit handler for layout_paragraphs_block_configure_form().
 */
function layout_paragraphs_block_configure_form_submit($form, &$form_state) {
  /** @var BlockParagraphs $block */
  $block = $form_state['block'];
  /** @var Layout $layout */
  $layout = layout_paragraphs_get_layout_tempstore($form_state['layout']->name);
  if (!$layout) {
    $layout = $form_state['layout'];
  }
  /** @var LayoutStyle $style */
  $style = $form_state['style'];
  /** @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];

  // Save the block settings.
  $block->formSubmit($form, $form_state);

  // Save the style settings.
  if (empty($form_state['values']['style_settings'])) {
    $form_state['values']['style_settings'] = array();
  }
  $style->formSubmit($form, $form_state);
  $block->style = $style;

  // Generate a UUID for the block.
  if (empty($block->uuid)) {
    $paragraphs_item_id = $block->paragraphs_item->id() ?? 0;
    $block->uuid = layout_paragraphs_get_block_uuid($paragraphs_item_id);
  }

  // Temporarily store any unfinished blocks (those that do not yet have a
  // position but were just added) in the layout so it can be retrieved. See
  // layout_paragraphs_tempstore_block_load().
  if (!empty($form_state['triggering_element']['#save_in_progress'])) {
    $layout->in_progress['region_name'] = $form_state['values']['region'];
    $layout->in_progress['block'] = $block;
    $layout->in_progress['renderer'] = $renderer;
  }
  // Insert new blocks into the layout.
  else {
    $layout->content[$block->uuid] = $block;
    if ($form_state['values']['region']) {
      $layout->setBlockPosition($block->uuid, $form_state['values']['region']);
    }
    // Clean up the in_progress value now that we're fully done.
    $layout->in_progress = NULL;
  }
  // Cleanup if needed for multi-value fields to remove empty slot.
  $paragraph_type = $block->paragraphs_item->bundle;
  $paragraph_fields = field_info_instances('paragraphs_item', $paragraph_type);
  foreach ($paragraph_fields as $field_name => $field_instance) {
    if (isset($block->paragraphs_item->{$field_name}[LANGUAGE_NONE][0]['fid']) || $block->paragraphs_item->{$field_name}[LANGUAGE_NONE][0]['fid'] === null) {
      foreach ($block->paragraphs_item->{$field_name}[LANGUAGE_NONE] as $delta => $item) {
        if (empty($item['fid'])) {
          unset($block->paragraphs_item->{$field_name}[LANGUAGE_NONE][$delta]);
        }
      }
      // Re-index the array.
      $block->paragraphs_item->{$field_name}[LANGUAGE_NONE] = array_values($block->paragraphs_item->{$field_name}[LANGUAGE_NONE]);
    }
  }
  layout_paragraphs_reset_caches();
  tempstore_set('layout_paragraphs_block_tempstore_link', $block->uuid, $layout->name, time() + 60 * 60 * 24);
  layout_paragraphs_set_layout_tempstore($layout);
}

/**
 * Helper function to get the UUID for a Paragraphs block for a particular
 * Paragraphs item id.
 *
 * @param int|string $paragraphs_item_id
 *   The Paragraphs item ID for which to get the UUID.
 *
 * @return string
 *   The UUID for the Paragraphs block, or a newly generated UUID if the
 *   Paragraphs item ID is not set.
 */
function layout_paragraphs_get_block_uuid($paragraphs_item_id = 0) {
  $uuid = '';
  // If the Paragraphs item ID is not set, generate a new UUID.
  if (!empty($paragraphs_item_id)) {
    // Get the UUID from the layout_paragraphs_blocks table.
    $uuid = db_query('SELECT uuid FROM {layout_paragraphs_blocks} WHERE paragraphs_item_id = :paragraphs_item_id', array(':paragraphs_item_id' => $paragraphs_item_id))->fetchField();
  }
  if (empty($uuid)) {
    $uuid = (new Uuid())->generate();
  }
  return $uuid;
}

/**
 * Menu callback; Remove a block from a layout.
 */
function layout_paragraphs_block_remove_page(Layout $layout, Block $block, $renderer_name) {
  if (backdrop_get_token('layout-' . $layout->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  $commands = array();
  // Save this information for later use, when the block itself is no longer
  // available but we still need to delete the Paragraphs item.
  $layout->in_progress['deleted_blocks'][] = $block;
  $layout->removeBlock($block->uuid);
  // Update the tempstore to account for the removed block.
  layout_paragraphs_set_layout_tempstore($layout);

  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#layout-editor-block-' . $block->uuid);
    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Block "@title" removed.', array('@title' => $block->getAdminTitle())));
    backdrop_goto();
  }
}

/**
 * Menu callback; Set a block as disabled.
 */
function layout_paragraphs_block_disable_page(Layout $layout, Block $block, $renderer_name) {
  if (backdrop_get_token('layout-' . $layout->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  $uuid = $block->uuid;
  $layout->content[$uuid]->status = $block->status ? BLOCK_DISABLED : BLOCK_ENABLED;
  layout_paragraphs_set_layout_tempstore($layout);

  $renderer = layout_create_renderer($renderer_name, $layout);

  $block->status = $layout->content[$uuid]->status;
  $replace_block = $renderer->renderBlock($block);

  if (backdrop_is_ajax()) {
    if ($message = layout_paragraphs_locked_message($layout, 'layout')) {
      backdrop_set_message($message, 'warning');
      $commands[] = ajax_command_remove('#messages');
      $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
    }

    $commands[] = ajax_command_replace('#layout-editor-block-' . $block->uuid, $replace_block);
    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Block "@title" disabled.', array('@title' => $block->getAdminTitle())));
    // @todo: Redirect to the current entity edit page.
    // backdrop_goto('admin/structure/layouts/manage/' . $layout->name);
  }

}/**
 * AJAX callback to update the underlying layout form after updating a
 * Layout Paragraphs block.
 */
function layout_paragraphs_block_configure_ajax_update($form, &$form_state) {
  /** @var Layout $layout */
  $layout = $form_state['layout'];
  // Get the latest version of the layout from tempstore.
  $tempstore_layout = layout_paragraphs_get_layout_tempstore($layout->name);
  if ($tempstore_layout) {
    $layout = $tempstore_layout;
  }
  /** @var Block $block */
  $block = $form_state['block'];
  /** @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];
  $added_to_region = $block->is_new ? ($layout->in_progress['region_name'] ?? $form_state['values']['region']) : FALSE;

  // Display error messages in the form if any.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array(
      'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
    ));
  }
  // If a new block, add to the bottom of the region.
  elseif ($added_to_region) {
    $commands[] = ajax_command_close_modal_dialog();
    $commands[] = ajax_command_remove('#layout-editor-block-' . $block->uuid);
    $commands[] = ajax_command_append('#layout-editor-region-' . $added_to_region . ' .layout-editor-region-content', $renderer->renderBlock($block));
  }
  // If editing a block, replace the block with its placeholder.
  else {
    $commands[] = ajax_command_close_modal_dialog();
    $commands[] = ajax_command_replace('#layout-editor-block-' . $block->uuid, $renderer->renderBlock($block));
  }

  // Update the messages area.
  if ($message = layout_paragraphs_locked_message($layout)) {
    backdrop_set_message($message, 'warning');
  }
  $commands[] = ajax_command_remove('#messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));

  // Update form_state and the layout's tempstore with the new block data.
  $uuid = $block->uuid;
  $layout->content[$uuid] = $block;
  layout_paragraphs_set_layout_tempstore($layout);
  $form_state['layout'] = $layout;

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Menu callback; Enable or disable a layout paragraphs template.
 */
function layout_paragraphs_template_toggle_enabled($template_name, $status) {
  if (backdrop_get_token('layout-paragraphs-template-' . $template_name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }
  $template_info = layout_get_layout_template_info($template_name, NULL, 'paragraphs');
  if (!$template_info) {
    return MENU_NOT_FOUND;
  }

  $config = config('layout_paragraphs.settings');
  $disabled_templates = $config->get('disabled_templates') ?? [];
  $errors = FALSE;

  if (!$status) {
    if (!in_array($template_name, $disabled_templates)) {
      $disabled_templates[] = $template_name;
    }
    // Check which layout templates are in use.
    $templates_in_use = _layout_paragraphs_get_templates_in_use();
    if (in_array($template_name, $templates_in_use)) {
      $errors = TRUE;
    }
    if ($errors) {
      backdrop_set_message(t('The "@title" layout template is currently in use and may not be disabled.', array('@title' => $template_info['title'])), 'error');
    }
    else {
      backdrop_set_message(t('Layout template "@title" has been disabled.', array('@title' => $template_info['title'])));
    }
  }
  else {
    $disabled_templates = array_diff($disabled_templates, array($template_name));
    backdrop_set_message(t('Layout template "@title" has been enabled.', array('@title' => $template_info['title'])));
  }

  if (!$errors) {
    $config->set('disabled_templates', $disabled_templates);
    $config->save();
  }

  backdrop_goto('admin/structure/paragraphs/layouts');
}

/**
 * Prepare a message for when a Layout Paragraphs item is updated or locked.
 */
function layout_paragraphs_locked_message($item) {
  $message = t('This form has unsaved changes. Click "@button" to make changes permanent or "Cancel" to discard changes.', array(
    '@button' => t('Save'),
  ));
  return $message;
}

/**
 * Helper to load paragraph blocks into a layout and update the tempstore.
 *
 * @param Layout $layout
 *   The layout object to load blocks into.
 * @param string $source
 *   The source to load blocks from, either 'database' or 'tempstore'.
 * @param string $lp_id
 *   The ID of the layout paragraph to load blocks from, in the case of loading
 *   blocks from the database.
 *
 * @return bool
 *   TRUE if the source was valid and tempstore was updated, FALSE otherwise.
 */
function _layout_paragraphs_add_paragraphs_to_layout(Layout $layout, string $source = 'database', string $lp_id = '') {
  $update_needed = TRUE;
  $skip_tempstore_update = FALSE;
  $orphan_block_count = 0;
  $layout_template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
  // @todo: We may be able to make this auto replacement more intuitive, e.g.
  // if the layout has a region with the same machine name, leave it there.
  // Right now everything flows into the first region.
  $default_region = _layout_paragraphs_get_default_region($layout_template_info['regions'] ?? []);
  list($entity_type, $bundle, $field_name, $entity_id) = explode('.', $layout->name);
  switch ($source) {
    case 'database':
      $placed_blocks = array();
      if (empty($lp_id)) {
        if (is_numeric($entity_id)) {
          $query = db_select('layout_paragraphs', 'lp')
            ->fields('lp')
            ->condition('lp.entity_type', $entity_type)
            ->condition('lp.bundle', $bundle)
            ->condition('lp.field_name', $field_name)
            ->condition('lp.entity_id', $entity_id);
          $result = $query->execute()->fetchAssoc();
          $lp_id = $result['id'];
        }
      }
      if (!empty($lp_id)) {
        // Add specifically-placed blocks.
        $query = db_select('layout_paragraphs_blocks', 'lpb')
          ->fields('lpb')
          ->condition('lpb.lp_id', $lp_id)
          ->orderBy('lpb.block_index');
        $result = $query->execute();
        foreach ($result as $block_info) {
          $placed_blocks[$block_info->paragraphs_item_id] = $block_info;
        }
      }
      if (!empty($entity_id) && is_numeric($entity_id)) {
        $entity = entity_load($entity_type, $entity_id);
        $paragraphs_field_values = field_get_values($entity, $field_name);
        foreach ($paragraphs_field_values as $delta => $paragraphs_field_value) {
          $paragraph_id = $paragraphs_field_value;
          $region_id = $placed_blocks[$paragraph_id]->region_id ?? '';
          if (!isset($placed_blocks[$paragraph_id]) || empty($region_id) || !isset($layout->positions[$region_id])) {
            // Consider any Paragraphs items that are:
            // - not explicitly placed, for example if the Layout UI was turned
            //   on after content was created with the Embedded widget.
            // - explicitly placed but in a region that is not in the current
            //   layout.
            backdrop_set_message('Expecting region: ' . $region_id . ' but it was not found.');
            $orphan_block_count++;
            $block_info = new stdClass();
            // $block_info->uuid = 'layout_paragraphs:' . $paragraph_id;
            $block_info->uuid = layout_paragraphs_get_block_uuid($paragraph_id);
            $block_info->block_index = $delta;
            $block_info->block_data = json_encode(array(
              'paragraphs_item_id' => $paragraph_id,
              'layout_name' => $layout->name,
            ));
            $block_info->region_id = $default_region;
            $block_info->orphan = TRUE;
            $placed_blocks[$paragraph_id] = $block_info;
          }
        }
      }
      foreach ($placed_blocks as $paragraph_id => $block_info) {
        $region_id = $block_info->region_id ?? $default_region;
        $block_paragraph = entity_load('paragraphs_item', $paragraph_id);
        if (!empty($block_paragraph)) {
          $paragraphs_block_key = 'layout_paragraphs:' . $block_paragraph->bundle;
          $block_data = [
            'paragraphs_item_id' => $paragraph_id,
            'layout_name' => $layout->name,
            'uuid' => $block_info->uuid,
            'orphan' => !empty($block_info->orphan),
          ];
          $block = new BlockParagraphs($paragraphs_block_key, $block_data);
          if (!empty($block_paragraph->status)) {
            // Ensure block is not disabled.
            $block->status = TRUE;
          }
          else {
            // Set block status to disabled because the paragraph is disabled.
            $block->status = FALSE;
          }
        }
        else {
          // Set a broken block.
          $block = new BlockBroken($paragraphs_block_key ?? 'layout_paragraphs:unknown', (array) json_decode($block_info->block_data));
          $block->status = FALSE;
        }
        $layout->content[$block_info->uuid] = $block;
        $layout->positions[$region_id][] = $block_info->uuid;

        // Move the block into the correct position.
        $position = $block_info->block_index;
        if (is_numeric($position)) {
          $reordered_positions = array();
          foreach ($layout->positions[$region_id] as $n => $uuid) {
            if ((int) $position == (int) $n) {
              $reordered_positions[] = $block_info->uuid;
            }
            if (!in_array($uuid, $reordered_positions)) {
              $reordered_positions[] = $uuid;
            }
          }
          $layout->positions[$region_id] = $reordered_positions;
        }
      }
      if ($orphan_block_count > 0) {
        backdrop_set_message(t('There are @count orphaned blocks in this layout, placed automatically in the default region.', array('@count' => $orphan_block_count)), 'warning');
      }
      // Update the layout's tempstore with the blocks loaded from the database.
      layout_paragraphs_set_layout_tempstore($layout);
      break;

    case 'tempstore':
      $old_layout = layout_paragraphs_get_layout_tempstore($layout->name);
      if (!empty($old_layout)) {
        $old_layout_blocks = $old_layout->content;
      }
      else {
        // There is no tempstore data, so let's try loading from the database
        // and return that result without updating it again here (since it will
        // be updated when loaded in this call).
        $update_needed = _layout_paragraphs_add_paragraphs_to_layout($layout, 'database', $lp_id);
        $skip_tempstore_update = TRUE;
      }
      break;

    default:
      $update_needed = FALSE;
      break;
  }
  if ($update_needed && !$skip_tempstore_update && !empty($old_layout_blocks)) {
    foreach ($old_layout_blocks as $uuid => $block) {
      $newly_placed = !isset($layout->content[$uuid]);
      $layout->content[$uuid] = $block;
      if ($newly_placed) {
        $layout->setBlockPosition($uuid, $default_region);
      }
    }
    layout_paragraphs_set_layout_tempstore($layout);
  }
  return $update_needed;
}

/**
 * Helper to get an ID for a layout based on blocks that have been placed
 * within it and have saved tempstore breadcrumbs. This is used to link the
 * layout and blocks across various forms and submissions that end up
 * resetting data within the form. If we can optimize this better and make this
 * unnecessary, we should do so.
 */
function _layout_paragraphs_get_layout_id_from_block_tempstore(array $block_positions) {
  $entity_id = NULL;
  foreach ($block_positions as $position_key => $blocks) {
    $blocks_array = explode(',', $blocks);
    foreach ($blocks_array as $block_uuid) {
      // If we've edited the layout previously, blocks have been storing
      // temporary linkages to the layout name so we can retrieve them.
      // @see layout_paragraphs_block_configure_form_submit()
      $layout_name = tempstore_get('layout_paragraphs_block_tempstore_link', $block_uuid);
      if (!empty($layout_name)) {
        list(,,, $entity_id) = explode('.', $layout_name);
        break;
      }
    }
    if (!empty($entity_id)) {
      break;
    }
  }
  return $entity_id;
}
