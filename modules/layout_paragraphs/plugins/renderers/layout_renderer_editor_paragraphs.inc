<?php
/**
 * @file
 * Class file to control the main Layout Paragraphs editor.
 */

/**
 *
 */
class LayoutRendererEditorParagraphs extends LayoutRendererEditor {

  /**
   * Information about the Paragraphs field instance.
   */
  private $paragraphs_field_info_instance;

  /**
   * {@inheritdoc}
   */
  public function __construct(Layout $layout, array $renderer_plugin) {
    $template_id = $layout->layout_template ?? 'default';

    $template_info = layout_paragraphs_get_layout_template_info($template_id);
    $template_data = layout_paragraphs_tempstore_load($template_id);

    foreach ($template_data->rows as $position => $info) {
      $layout->positions[$position] = array();
      $template_info['regions'][$position] = $position;
    }

    $this->admin = TRUE;
    $this->layout = &$layout;
    $this->layout_template_info = $template_info;
    $this->plugin = $renderer_plugin;
    $this->paragraphs_field_info_instance = _layout_paragraphs_get_field_instance($layout->name);
    // @todo: Add content/blocks?
  }

  /**
   * {@inheritdoc}
   */
  public function addMeta() {
    parent::addMeta();
    backdrop_add_library('system', 'ui.sortable');
    backdrop_add_js(backdrop_get_path('module', 'layout') . '/js/layout.admin.js');
    backdrop_add_css(backdrop_get_path('module', 'layout') . '/css/layout.admin.css');
  }

  /**
   * {@inheritdoc}
   */
  public function render() {
    $this->addMeta();

    $output = '<div class="layout--paragraphs-edit-main">';
    $output .= $this->renderLayout();
    $output .= '</div>';

    return $output;
  }

  /**
   * Perform display/layout-level render operations.
   *
   * This method triggers all the inner block/region rendering processes, passes
   * that to the layout plugin's theme callback, and returns the rendered HTML.
   *
   * If display-level caching is enabled and that cache is warm, this method
   * will not be called.
   *
   * @return string
   *   The HTML string representing the entire rendered, themed layout.
   */
  public function renderLayout() {
    if (empty($this->prep_run)) {
      $this->prepare();
    }

    $this->renderBlocks();

    $this->renderRegions();

    module_load_include('inc', 'layout', 'layout.theme');
    if (empty($this->layout_template_info['template'])) {
      $theme = 'layout__' . $this->layout->layout_template;
    }
    else {
      $theme = str_replace('-', '_', $this->layout_template_info['template']);
    }
    $layout_details = array(
      'content' => $this->rendered['regions'],
      'settings' => $this->layout->settings,
      'layout' => $this->layout,
      'layout_template_info' => $this->layout_template_info,
      'renderer' => $this,
      'admin' => $this->admin,
    );
    $this->rendered['layout'] = theme($theme, $layout_details);
    return $this->prefix . $this->rendered['layout'] . $this->suffix;
  }

  /**
   * {@inheritdoc}
   */
  public function renderRegion($region_id, $blocks) {
    $output = '';
    $content = implode('', $blocks);
    $region_buttons = $this->getRegionLinks($region_id);

    $output = '<div class="layout-editor-region" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
    $output .= '<div class="layout-editor-region-title clearfix">';
    $output .= '<h2 class="label">' . check_plain($this->layout_template_info['regions'][$region_id]) . '</h2>';
    $output .= $region_buttons;
    $output .= '</div>';
    $output .= '<div class="layout-editor-region-content">' . $content . '</div>';
    $output .= '</div>';

    return $output;
  }

  /**
   * Render the Page Title region in the Layout Editor.
   */
  public function renderTitleRegion($region_id) {
    $region_buttons = $this->getRegionLinks($region_id);
    $description = layout_get_title_description($this->layout);

    $output  = '<div class="layout-editor-block" id="layout-editor-title" data-region-name="' . $region_id . '">';
    $output .= '  <div class="layout-editor-block-title clearfix">';
    $output .= '    <span class="text">' . check_plain($this->layout_template_info['regions'][$region_id]) . '</span>';
    $output .= '    <span class="description">' . $description . '</span>';
    $output .= '    <span class="buttons">' . $region_buttons . '</span>';
    $output .= '  </div>';
    $output .= '</div>';

    return $output;
  }

  /**
   * {@inheritdoc}
   */
  public function renderBlock($block) {
    $aria_label = $this->getAriaLabel($block);
    $buttons = $this->getBlockLinks($block);
    $content = $block->getAdminPreview();

    $attributes = array(
      'tabindex' => '0',
      'aria-label' => $aria_label,
      'class' => array('layout-editor-block clearfix'),
      'id' => 'layout-editor-block-' . $block->uuid,
      'data-block-id' => $block->uuid,
    );

    $block_admin_title = $block->getAdminTitle();
    if (!$block->status) {
      $attributes['class'][] = 'layout-editor-block-disabled';
      $block_admin_title = t('!title (Disabled)', array('!title' => $block_admin_title));
    }

    $output = '<div ' . backdrop_attributes($attributes) . '>';
    $output .= '<div class="layout-editor-block-title clearfix">';
    $output .= '<span class="handle"></span><span class="text">' . $block_admin_title . '</span>';
    if ($buttons) {
      $output .= '<span class="buttons">' . $buttons . '</span>';
    }
    $output .= '</div>'; // layout-block-title.
    $output .= '<div tabindex="0" class="layout-editor-block-content">' . $content . '</div>';
    $output .= '</div>'; // layout-block.

    return $output;
  }

  /**
   * Overrides LayoutRendererStandard::prepareBlocks() to process Layout
   * Paragraphs blocks.
   */
  public function prepareBlocks(array $blocks) {
    // Use local variables as writing to them is very slightly faster.
    $first = $normal = $last = array();

    // Prepare the list of blocks to be rendered.
    foreach ($blocks as $uuid => $block) {
      // Check access on each block.
      if (empty($this->admin)) {
        // Assign contexts to each block.
        $layout_contexts = $this->layout->getContexts();

        $has_contexts = TRUE;
        $required_contexts = $block->getRequiredContexts();
        if ($required_contexts) {
          $has_contexts = $this->layout->hasContexts($required_contexts);
          if ($has_contexts) {
            $block->setContexts($layout_contexts);
          }
        }
        elseif ($block->conditions) {
          $block->setContexts($layout_contexts);
        }

        // Then check access on the block.
        if (!$has_contexts || !$block->checkAccess()) {
          continue;
        }
      }

      // Let the block prepare any data before rendering.
      $block->prepare();

      $block_info = layout_get_block_info($block->module, $block->delta);

      // If this block wants to render last, add it to the $last array. We allow
      // this because some blocks need to be rendered after other blocks,
      // primarily so they can do things like the leftovers of forms.
      if (!empty($block_info['render last'])) {
        $last[$uuid] = $block;
      }
      // If it wants to render first, add it to the $first array. This is used
      // by blocks that need to do some processing before other blocks are
      // rendered.
      elseif (!empty($block_info['render first'])) {
        $first[$uuid] = $block;
      }
      // Otherwise, render it in the normal order.
      else {
        $normal[$uuid] = $block;
      }
    }

    $this->prepared['blocks'] = $first + $normal + $last;
    return $this->prepared['blocks'];
  }

  /**
   * {@inheritdoc}
   */
  public function renderBlocks() {
    $this->rendered['blocks'] = array();
    foreach ($this->prepared['blocks'] as $uuid => $block) {
      $block->uuid = $uuid;
      $this->rendered['blocks'][$uuid] = $this->renderBlock($block);
    }
    return $this->rendered['blocks'];
  }

  /**
   * Returns an aria label for a block.
   */
  public function getAriaLabel($block) {
    return t('Draggable block !title', array('!title' => strip_tags($block->getAdminTitle())));
  }

  /**
   * Short-cut to generate a URL path to a particular action.
   *
   * @param string $command
   *   The command to which the URL should point, usually "configure-block",
   *   "remove-block", or "add-block".
   * @param ...
   *   Any additional parameters to be tacked on the end of the URL.
   *
   * @return string
   *   A Backdrop menu path.
   */
  public function getUrl($command) {
    $args = func_get_args();
    $command = array_shift($args);
    $url = 'admin/structure/paragraphs/layouts/manage/' . $this->layout->name . '/' . $command . '/' . $this->plugin['name'];
    if ($args) {
      $url .= '/' . implode('/', $args);
    }
    return $url;
  }

  /**
   * Render the links to display when editing a region.
   */
  protected function getRegionLinks($region_id) {
    $links = array();
    // @todo: Check permissions for adding/configuring paragraphs.
    $title = $this->paragraphs_field_info_instance['settings']['title'] ?? t('paragraph');
    $links['add'] = array(
      'title' => t('Add !title', array('!title' => $title)),
      'href' => $this->getUrl('add-block', $region_id),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array(
          'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
        )),
      ),
    );
    $links['configure'] = array(
      'title' => t('Configure region'),
      'href' => $this->getUrl('configure-region', $region_id),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array(
          'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
        )),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }

  /**
   * Render the links to display when editing a block.
   */
  protected function getBlockLinks($block) {
    $links = array();

    if (!$block->status) {
      $links['enable'] = array(
        'title' => t('Enable'),
        'href' => $this->getUrl('toggle-block-status', $block->uuid),
        'query' => array('token' => backdrop_get_token('layout-' . $this->layout->name)),
        'attributes' => array(
          'class' => array('enable-block', 'use-ajax'),
        ),
      );
    }

    // Only render the "Configure" link if the block is not broken.
    if (!is_a($block, 'BlockBroken')) {
      $links['configure'] = array(
        'title' => t('Configure'),
        'href' => $this->getUrl('configure-block', $block->uuid),
        'attributes' => array(
          'class' => array('use-ajax'),
          'data-dialog' => TRUE,
          'data-dialog-options' => json_encode(array(
            'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
          )),
        ),
      );
    }

    if ($block->module == 'system' && $block->delta == 'page_components' && ($block->childDelta == 'title' || $block->childDelta == 'title_combo')) {
      $links['title'] = array(
        'title' => t('Page title settings'),
        'href' => $this->getUrl('edit-title', 'title'),
        'attributes' => array(
          'class' => array('use-ajax'),
          'data-dialog' => TRUE,
          'data-dialog-options' => json_encode(array(
            'classes' => array('ui-dialog' => 'layout-paragraphs-dialog'),
          )),
        ),
      );
    }

    $links['remove'] = array(
      'title' => t('Remove'),
      'href' => $this->getUrl('remove-block', $block->uuid),
      'query' => array('token' => backdrop_get_token('layout-' . $this->layout->name)),
      'attributes' => array(
        'class' => array('remove-block', 'use-ajax'),
      ),
    );

    if ($block->status) {
      $links['disable'] = array(
        'title' => t('Disable'),
        'href' => $this->getUrl('toggle-block-status', $block->uuid),
        'query' => array('token' => backdrop_get_token('layout-' . $this->layout->name)),
        'attributes' => array(
          'class' => array('disable-block', 'use-ajax'),
        ),
      );
    }

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }
}
