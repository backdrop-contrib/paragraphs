<?php
/**
 * @file
 */

/**
 * Select a Layout Paragraphs template from the options.
 */
function layout_paragraphs_select_template($field, $instance, $template_options, $default = 'default') {
  $form = [];
  $form['#field'] = $field;
  $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

  foreach ($template_options as $template_option) {
    $pause = 1;
  }

  $form['#attributes'] = array(
    'class' => array('layout-paragraphs-settings-form'),
  );

  $form['layout_paragraphs_templates'] = array(
    '#title' => t('Layout template'),
    '#type' => 'radios',
    '#default_value' => $default,
    '#options' => array(),
    '#wrapper_attributes' => array(
      'class' => array('clearfix', 'layout-options'),
    ),
    '#after_build' => array(
      'layout_paragraphs_select_template_after_build',
    ),
    '#required' => TRUE,
  );

  foreach ($template_options as $template_name => $template_info) {
    $form['layout_paragraphs_templates']['#options'][$template_name] = $template_info->title;
    if ($template_name == $default) {
      // $form['layout_paragraphs_templates']['#options'][$template_name]['#attributes']['data-default'] = 'default';
    }
    // $form['layout_paragraphs_templates'][$template_name]['#description'] = theme('layout_paragraphs_template_option', array('template_info' => $template_info));
    $form['layout_paragraphs_templates'][$template_name]['#description'] = $template_info->preview;
  }

  $form['layout_paragraphs_templates']['submit'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-paragraphs-templates-select'),
    ),
    'button' => array(
      '#type' => 'submit',
      '#value' => t('Change layout template'),
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'layout_paragraphs_select_template_change_ajax',
        'wrapper' => 'ajax-layout-paragraphs-templates',
        'effect' => 'fade',
      ),
      '#submit' => array(
        'layout_paragraphs_select_template_change_submit',
      ),
    ),
    '#weight' => 100,
  );

  return $form;
}

/**
 *
 */
function layout_paragraphs_select_template_change_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  return $form;
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
 * After build function to mark default option, which is used by CSS to hide
 * the submit button, if the default value continues to be selected.
 *
 * @param [type] $form
 * @param [type] $form_state
 * @return void
 */
function layout_paragraphs_select_template_after_build($element, $form_state) {
  $default_template = $element['#default_value'];
  $element[$default_template]['#attributes']['data-default'] = 'true';
  return $element;
}

/**
 * Ajax handler for layout_paragraphs_select_template() to change the active
 * Layout Paragraphs template.
 *
 * @param array $form
 * @param array $form_state
 * @return void
 */
function layout_paragraphs_select_template_change_ajax($form, &$form_state) {
  $new_template_name = $form_state['values']['layout_paragraphs_templates'];
  $form_element = $form['layout_paragraphs'];
  $field = $form_element['select_template']['#field'];
  $templates = &$form_element['select_template']['layout_paragraphs_templates'];
  $templates['#default_value'] = $new_template_name;
  foreach (element_children($templates) as $template_name) {
    if ($template_name == $new_template_name) {
      $templates[$template_name]['#attributes']['data-default'] = 'true';
    }
    else {
      unset($templates[$template_name]['#attributes']['data-default']);
    }
  }
  $entity = $form_state[$form['#entity_type']] ?? NULL;
  if (empty($entity) && $form['#entity_type'] == 'taxonomy_term' && !empty($form['#term'])) {
    // Special case due to mismatch between entity type and form key.
    $entity = $form_state['term'];
  }
  $entity_id = (!empty($entity->id())) ?? '';
  $layout_name = _layout_paragraphs_get_layout_name($form['layout_paragraphs']['#instance'], $entity_id);
  // @todo adjust this refreshing of the layout and shifting paragraphs/regions;
  // load the old layout and process all regions/blocks to new layout.
  $old_layout = tempstore_get('layout_paragraphs', $layout_name);

  $config = array(
    'is_new' => TRUE,
    'layout_template' => $new_template_name,
    'layout_template_type' => 'paragraphs',
    'is_full_page' => FALSE,
    'name' => $form['#entity_type'] . '.' . $form['#bundle'] . '.' . $field['field_name'] . '.' . $entity_id,
  );
  $layout = new Layout($config);
  $layout->setContexts('paragraphs', layout_paragraphs_context());
  // $layout_renderer = layout_get_renderer_info('paragraphs_editor');
  // $renderer = new LayoutRendererEditorParagraphs($layout, $layout_renderer);
  $layout_form = $layout_form_state = [];
  $form_element['layout'] = layout_paragraphs_content_form($layout_form, $layout_form_state, $layout, 'paragraphs_editor');
  $form_element['layout']['#layout']->renderer_name = 'paragraphs_editor';
  return $form_element;
}

/**
 * Returns the paragraphs context, which is available for Layout Paragraphs only.
 *
 * @return EntityLayoutContext
 *   The layout context for paragraphs.
 *
 * @since 1.17.5 Function added.
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
function layout_paragraphs_content_form($form, &$form_state, Layout $layout, $renderer_name = NULL) {
  $layout->setContexts('paragraphs', layout_paragraphs_context());
  if (!isset($form_state['layout'])) {
    $form_state['layout'] = $layout;
  }
  if ($renderer_name) {
    $renderer = layout_create_renderer($renderer_name, $layout);
  }
  else {
    $renderer = layout_create_renderer('editor', $layout);
  }

  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = $renderer;
  }

  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var LayoutRendererEditorParagraphs $renderer */
  $renderer = $form_state['renderer'];

  // Although layout is already set in $form_state, we need this to make it
  // available to the theme layer.
  $form['#layout'] = $layout;

  $form['#attached'] = array(
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

  $form['messages'] = array(
    '#markup' => $layout->locked ? '<div class="messages warning">' . layout_paragraphs_locked_message($layout) . '</div>' : '',
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $form['content'] = array('#tree' => TRUE);
  $form['content']['block'] = array('#tree' => TRUE);
  $blocks_by_region = [];
  $lp_id = _layout_paragraphs_get_lp_id($layout->name);
  // @todo: populate blocks here? From db or tempstore?
  /*
  $blocks = db_select('layout_paragraphs_blocks', 'lpb')
    ->fields('lpb')
    ->condition('lpb.lp_id', $lp_id)
    ->execute()
    ->fetchAllAssoc('uuid');
  foreach ($blocks as $block) {
    $blocks_by_region[$block->region_id][] = $block->uuid;
  }
  */

  foreach ($renderer->layout_template_info['regions'] as $region_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($blocks_by_region[$region_id])) {
      $blocks_by_region[$region_id] = [];
    }

    $form['content']['positions'][$region_id] = array(
      // Use 'hidden' instead of 'value' so the JS can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', $blocks_by_region[$region_id]),
    );
  }

  // @todo: This is a hack to get the layout to render correctly. It can
  // probably be improved.
  // $renderer->layout->content = $form['content']['positions'];
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );
  tempstore_set('layout_paragraphs', $layout->name, $layout, 604800);
  return $form;
}

/**
 * Validate the layout settings form.
 */
function layout_paragraphs_content_form_validate($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];

  // Ensure that no block is positioned more than once. This is merely a sanity
  // check, it should not occur within the normal interface.
  $all_uuids = array();
  foreach ($layout->positions as $uuids) {
    foreach ($uuids as $uuid) {
      if (in_array($uuid, $all_uuids)) {
        form_error($form['content']['positions'], t('The "@block" block has incorrectly been positioned multiple times. This may indicate an incorrectly saved layout configuration.', array('@block' => $layout->content[$uuid]->getAdminTitle())));
      }
      $all_uuids[] = $uuid;
    }
  }

  // Let the layout do any specific validations.
  $layout->formValidate($form, $form_state);
}

/**
 * Handle form submission of the display content editor.
 *
 * This reads the location of the various blocks from the form, which will
 * have been modified via JavaScript, rearranges them and then saves
 * the display.
 */
function layout_paragraphs_content_form_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = &$form_state['layout'];

  // @todo: How is this different than layout_paragraphs_edit_content_submit,
  // and is it ever called?

  // Update the block positions based on the hidden position elements.
  if (!empty($form_state['values']['content']['positions'])) {
    $new_content = array();
    $new_positions = array();

    foreach ($form_state['values']['content']['positions'] as $region_id => $blocks) {
      $new_positions[$region_id] = array();
      if ($blocks) {
        $uuids = array_filter(explode(',', $blocks));
        foreach ($uuids as $uuid) {
          if (isset($layout->content[$uuid])) {
            $new_content[$uuid] = $layout->content[$uuid];
            $new_positions[$region_id][] = $uuid;
          }
        }
      }
    }
    $layout->content = $new_content;
    $layout->positions = $new_positions;
  }

  // Save title settings.
  if (isset($form_state['values']['title_display'])) {
    $layout->settings['title'] = $form_state['values']['title'];
    $layout->settings['title_display'] = $form_state['values']['title_display'];
  }

  // Let layouts save their own settings if needed.
  $layout->formSubmit($form, $form_state);

  // Save the entire layout to configuration.
  $layout->save();

  // Delete the tempstore copy of the layout.
  layout_clear_layout_tempstore($layout->name, 'paragraphs');

  backdrop_set_message(t('Layout content saved.'));
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

  $rows = array();
  $options = array();
  $default_template_name = 'default';
  $form_state['hidden_templates'] = array();
  $all_templates = layout_flexible_template_load_all('paragraphs');

  foreach ($all_templates as $template_name => $template) {
    // Build the default values list.
    $is_default = ($template_name == $default_template_name);
    (empty($template->hidden)) ? $template->hidden = 0 : $template->hidden = 1;
    $template_info = array(
      'preview' => (!empty($template->preview)) ? $template->preview : '',
      'title' => $template->title,
      'name' => $template_name,
    );

    // Build the list of possible options, either enabled or not hidden.
    if ($is_default || !$template->hidden) {
      $rows[$template_name] = array(
        'layout_paragraphs_template_info' => theme('layout_paragraphs_template_info', array(
          'template_info' => $template_info,
        )),
        'type' => t('Paragraphs'),
      );
      $rows[$template_name]['operations']['data'] = array(
        '#type' => 'dropbutton',
        '#links' => _layout_paragraphs_settings_get_group_operations($template_info, $is_default),
      );
    }
    // For disabled and hidden templates, save the fact they are disabled.
    elseif ($template->hidden) {
      $form_state['hidden_templates'][] = $template_name;
    }
  }

  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';
  // Empty table header for enable/disable is confusing (below); converted to
  // standard table for now.
  /*
  $form['templates'] = array(
    '#type' => 'tableselect',
    '#js_select' => FALSE,
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('There are no Layout Paragraphs templates.'),
    '#default_value' => $default,
    '#attributes' => array('class' => array('layout-list')),
  );
  */
  $form['templates'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are no Layout Paragraphs templates.'),
    '#attributes' => array('class' => array('layout-list')),
  );

  return $form;
}

  /**
   * Given a path, return a list of operations for a group of layouts.
   */
function _layout_paragraphs_settings_get_group_operations($template_info, $is_default) {
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
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => t('Layout Paragraphs'),
          'name' => 'layout.paragraphs.' . $template_info['name'],
        ),
      );
    }
    if (!$is_default) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/structure/paragraphs/layouts/' . $template_info['name'] . '/delete',
      );
      if (empty($template_info['hidden'])) {
        $links['disable'] = array(
          'title' => t('Disable'),
          'href' => 'admin/structure/layouts/settings/toggle/' . $template_info['name'] . '/disable',
          'query' => array('token' => backdrop_get_token('layout-template-' . $template_info['name'])),
        );
      }
      else {
        $links['enable'] = array(
          'title' => t('Enable'),
          'href' => 'admin/structure/layouts/settings/toggle/' . $template_info['name'] . '/enable',
          'query' => array('token' => backdrop_get_token('layout-template-' . $template_info['name'])),
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
 *   The machine name of the layout template, or `default` if there was no database
 *   entry.
 */
function _layout_paragraphs_determine_layout_template($entity_id, $entity_type, $bundle, $field_name, $available_templates) {
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
  /* @var LayoutFlexibleTemplate $layout_paragraphs_template */
  $layout_paragraphs_template = $form_state['flexible_template'];
  $layout_paragraphs_template->name = $form_state['values']['machine_name'];
  $layout_paragraphs_template->title = $form_state['values']['name'];
  $layout_paragraphs_template->description = $form_state['values']['description'];
  $layout_paragraphs_template->layout_template_type = 'paragraphs';
  $layout_paragraphs_template->is_full_page = FALSE;
  unset($layout_paragraphs_template->is_new);
  $layout_paragraphs_template->preview = _layout_paragraphs_template_generate_svg($layout_paragraphs_template);

  $layout_paragraphs_template->save();

  layout_paragraphs_tempstore_set($layout_paragraphs_template);
  cache()->delete('layout:paragraphs:config');
  backdrop_set_message(t('Layout paragraphs template "@title" saved.', array('@title' => $layout_paragraphs_template->title)));
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
    '#attributes' => array('class' => array('layout-flexible-add-row', 'button-secondary')),
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
  cache()->delete('layout:paragraphs:config');
  backdrop_set_message(t('Layout Paragraphs template "@title" saved.', array('@title' => $layout_paragraphs_template->title)));
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
 * @param $machine_name
 *   The machine name for a region.
 *
 * @return
 *   Whether or not the region name is taken.
 *
 * @see layout_paragraphs_template_edit_row_form()
 * @see form_validate_machine_name()
 */
function _layout_paragraphs_template_region_name_exists($machine_name, $element, $form_state) {
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

  layout_paragraphs_tempstore_set($form_state['flexible_template']);

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
  /* @var LayoutFlexibleTemplate $layout_paragraphs_template */
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
  layout_paragraphs_tempstore_set($layout_paragraphs_template);
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
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
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
  layout_paragraphs_tempstore_set($layout_paragraphs_template);
  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#flexible-row--' . $original_row);

    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Row "@title" removed.', array('@title' => $layout_paragraphs_template->title)));
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
  $question = t('Delete flexible template @title?', array('@title' => $layout_paragraphs_template->title));
  $description = t('This action cannot be undone.');
  backdrop_set_title($question, PASS_THROUGH);

  // Check if this template is in use.
  // @todo: Adjust this check for Layout Paragraphs usage
  $errors = array();
  foreach (layout_load_all() as $layout) {
    if ($layout->layout_template == $layout_paragraphs_template->name) {
      $errors[] = check_plain($layout->title);
    }
  }

  if (!empty($errors)) {
    backdrop_set_message(t('The "@title" layout template is currently in use in the following layouts and may not be deleted: !list', array('@title' => $layout_paragraphs_template->title, '!list' => theme('item_list', array('items' => $errors)))), 'error');
  }
  else {
    $form['description'] = array(
      '#markup' => $description,
    );
  }

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
  /* @var LayoutFlexibleTemplate $layout_paragraphs_template */
  $layout_paragraphs_template = &$form_state['flexible_template'];
  $layout_paragraphs_template->delete();

  backdrop_set_message(t('The layout template "@title" has been deleted.', array('@title' => $layout_paragraphs_template->title)));
  $form_state['redirect'] = 'admin/structure/paragraphs/layouts';
}

/**
 * Generate an SVG image to be used as a Paragraphs layout template thumbnail.
 * @todo If this goes in core it may be possible to use that and remove this.
 * @see https://github.com/backdrop/backdrop-issues/issues/4104
 *
 * @param $template
 *   A layout template object.
 *
 * @return string $svg
 *   The SVG of the generated thumbnail image.
 *
 *   This function only works with the GD toolkit. ImageMagick is not supported.
 */
function _layout_paragraphs_template_generate_svg($template) {
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
 * Save a block while editing with Layout Paragraphs.
 */
function layout_paragraphs_save_block($layout, $region_id, $uuid, $block_index) {
  $block = $layout->content[$uuid];

  $data = array(
    'nid' => $layout->node->nid,
    'uuid' => $uuid,
    'region_id' => $region_id,
    'block_index' => $block_index,
    'block_data' => json_encode($block),
  );
  $exists = db_query('SELECT count(uuid) FROM {layout_paragraphs_blocks} WHERE uuid = :uuid', array(':uuid' => $uuid))->fetchField();
  if ($exists) {
    backdrop_write_record('layout_paragraphs_blocks', $data, 'uuid');
  }
  else {
    backdrop_write_record('layout_paragraphs_blocks', $data);
  }
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
    'layout_paragraphs' => $blocks
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
 * Menu callback; Display a list of new Paragraph blocks to be added to a layout.
 */
function layout_paragraphs_block_add_page(Layout $layout, $renderer_name, $region_name, $paragraphs_block_key = NULL) {
  /* @var Layout $layout */
  $layout->setContexts('paragraphs', layout_paragraphs_context());
  $form_state['layout'] = $layout;
  $template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
  if (0 && !array_key_exists($region_name, $template_info['regions'])) {
    // @todo: Make sure this check works.
    return MENU_NOT_FOUND;
  }

  // If a Paragraphs block has been selected, return the block configuration form.
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
  $block_list = array();
  $field_instance = _layout_paragraphs_get_field_instance($layout->name);
  foreach ($blocks as $module => $module_blocks) {
    if (empty($module_blocks)) {
      continue;
    }
    foreach ($module_blocks as $delta => $block_info) {
      $delta_key = ($field_instance['settings']['allowed_bundles'][$delta] == -1) ? 'deselected' : 'selected';
      // @todo: Remove the commented section below if not needed.
      /* @var Block $parent_block */
      /*
      $parent_block = new BlockParagraphs($module . ':' . $delta, array());
      $children = $parent_block->getChildren();
      if (is_array($children)) {
        foreach ($children as $child_id => $child_info) {
          // Skip blocks that don't have context requirements met.
          $required_contexts = isset($child_info['required contexts']) ? $child_info['required contexts'] : array();
          if ($layout->hasContexts($required_contexts)) {
            $block_list[$module . ':' . $delta . ':' . $child_id] = $child_info;
          }
        }
      }
      else {
        $required_contexts = isset($block_info['required contexts']) ? $block_info['required contexts'] : array();
        // Skip blocks that don't have context requirements met.
        if ($layout->hasContexts($required_contexts)) {
          $block_list[$module . ':' . $delta] = $block_info;
        }
      }
      */
      $required_contexts = isset($block_info['required contexts']) ? $block_info['required contexts'] : array();
      // Skip blocks that don't have context requirements met.
      if ($layout->hasContexts($required_contexts)) {
        $block_list[$delta_key][$module . ':' . $delta] = $block_info;
      }
    }
  }
  if (empty($block_list['selected'])) {
    // If none are selected, all are allowed.
    $block_list = $block_list['deselected'];
  }
  else {
    $block_list = $block_list['selected'];
  }
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
            'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
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

  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = layout_create_renderer($renderer_name, $layout);
  }
  if (!isset($region_name)) {
    $region_name = $layout->getBlockPosition($block->uuid);
  }

  if (empty($block->uuid)) {
    $form['#title'] = t('Add @title', array('@title' => $block->getAdminTitle()));
  }
  else {
    $form['#title'] = t('Configure @title', array('@title' => $block->getAdminTitle()));
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
    '#value' => empty($block->uuid) ? t('Add block') : t('Update block'),
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
  /* @var BlockParagraphs $block */
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
  /* @var BlockParagraphs $block */
  $block = $form_state['block'];
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var LayoutStyle $style */
  $style = $form_state['style'];
  /* @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];
  $block_plugin = $block->plugin;

  // Save the block settings.
  $block->formSubmit($form, $form_state);

  // Save the style settings.
  $style->formSubmit($form['style']['style_settings'], $form_state);
  $block->style = $style;

  // Generate a UUID for the block.
  if (empty($block->uuid)) {
    $uuid = new Uuid();
    $block->uuid = $uuid->generate();
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


  // Save the Layout and Block/Paragraphs item to tempstore.
  $expire = REQUEST_TIME + settings_get('form_cache_expiration', config_get('system.core', 'form_cache_expiration'));
  tempstore_set('layout_paragraphs', $layout->name, $layout, $expire);
  // tempstore_set('layout_paragraphs_block', $block->uuid, $block, $expire);
}

/**
 * AJAX callback to update the underlying layout form after updating a
 * Layout Paragraphs block.
 */
function layout_paragraphs_block_configure_ajax_update($form, $form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];
  $added_to_region = $block->is_new ? (isset($layout->in_progress['region_name']) ? $layout->in_progress['region_name'] : $form_state['values']['region']) : FALSE;

  // Display error messages in the form if any.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
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

  // Remove the page title pseudo region if we're adding a Title block or a
  // Combo block.
  if ($block->module == 'system' && $block->delta == 'page_components' && ($block->childDelta == 'title' || $block->childDelta == 'title_combo')) {
    $commands[] = ajax_command_replace('#layout-editor-title', '<div id="layout-title-region-empty"></div>');
  }

  // Update the messages area.
  if ($message = layout_paragraphs_locked_message($layout)) {
    backdrop_set_message($message, 'warning');
  }
  $commands[] = ajax_command_remove('#messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
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
