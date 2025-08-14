<?php

/**
 * The standard render for a Layout Paragraphs display object.
 *
 * Given a fully-loaded Layout object, this class will turn its combination of
 * layout, blocks, and styles into HTML, invoking caching appropriately along
 * the way. Interacting with the renderer externally is very simple - just pass
 * it the display object and call the render() method:
 *
 * @code
 *   // Given that $layout is a fully loaded Layout object.
 *   $renderer = layout_get_renderer('standard', $layout)
 *   $html_output = $renderer->render();
 * @endcode
 */
class LayoutRendererStandardParagraphs extends LayoutRendererStandard {


  /**
   * Overrides LayoutRendererStandard::__construct().
   *
   * @param Layout $layout
   *   The layout object to be rendered.
   * @param array $plugin
   *   The definition of the renderer plugin.
   *
   * @throws LayoutMissingException
   */
  function __construct(Layout $layout, array $renderer_plugin) {
    $this->plugin = $renderer_plugin;
    $layout_template_info = layout_paragraphs_get_layout_template_info($layout->layout_template);
    $this->layout = &$layout;

    if (empty($layout_template_info)) {
      throw new LayoutMissingException(t('The layout plugin "@plugin" could not been found to display the "@title" layout.', array('@plugin' => $layout->layout_template, '@title' => $layout->title)));
    }
    else {
      $this->layout_template_info = $layout_template_info;
    }
  }

  /**
   * Overrides LayoutRendererStandard::prepareRegions().
   *
   * @return array
   *   An array of regions prepared for rendering.
   */
  function prepareRegions(array $region_block_list, array $settings) {
    // Initialize defaults to be used for regions without their own explicit
    // settings. Use display settings if they exist, else hardcoded defaults.
    $default = array(
      'style' => !empty($settings['style']) ? $settings['style'] : 'default',
      'style settings' => isset($settings['style_settings']['default']) ? $settings['style_settings']['default'] : array(),
    );
    $regions = array();
    if (empty($settings)) {
      // No region settings exist, init all with the defaults.
      foreach ($this->layout_template_info['regions'] as $region_id => $title) {
        // Ensure this region has at least an empty block array.
        $blocks = array();
        if (isset($region_block_list[$region_id])) {
          $blocks = $region_block_list[$region_id];
        }

        $regions[$region_id] = $default;
        $regions[$region_id]['uuids'] = $blocks;
      }
    }
    else {
      // Some settings exist; iterate through each region and set individually.
      foreach ($this->layout_template_info['regions'] as $region_id => $title) {
        // Ensure this region has at least an empty blocks array.
        $blocks = !empty($region_block_list[$region_id]) ? $region_block_list[$region_id] : array();

        if (empty($settings[$region_id]['style']) || $settings[$region_id]['style'] == -1) {
          $regions[$region_id] = $default;
        }
        else {
          $regions[$region_id]['style'] = $settings[$region_id]['style'];
        }

        $regions[$region_id]['style settings'] = isset($settings[$region_id]) ? $settings[$region_id] : array();
        $regions[$region_id]['uuids'] = $blocks;
      }
    }
    // Dump any unplaced blocks into the main region.
    $default_region = _layout_paragraphs_get_default_region($regions);
    foreach ($region_block_list as $region_uuid => $blocks) {
      if (!in_array($region_uuid, array_keys($regions))) {
        $regions[$default_region]['uuids'] += $blocks;
      }
    }

    $this->prepared['regions'] = $regions;
    return $this->prepared['regions'];
  }

  /**
   * Overrides LayoutRendererStandard::renderLayout().
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
    $this->rendered['layout'] = theme($theme, array(
      'content' => $this->rendered['regions'],
      'settings' => $this->layout->settings,
      'layout' => $this->layout,
      'layout_template_info' => $this->layout_template_info,
      'renderer' => $this,
      'admin' => $this->admin,
    ));
    return $this->prefix . $this->rendered['layout'] . $this->suffix;
  }

  /**
   * Overrides LayoutRendererStandard::renderRegions().
   *
   * @return array
   *   An array of rendered layout regions, keyed on the region name.
   */
  public function renderRegions() {
    $this->rendered['regions'] = array();

    // Loop through all layout regions, put all blocks that belong to the
    // current region in an array, then render the region. Primarily this
    // ensures that the blocks are arranged in the proper order.
    foreach ($this->prepared['regions'] as $region_id => $conf) {
      $region_blocks = array();
      foreach ($conf['uuids'] as $uuid) {
        // Only include blocks for region rendering if they had some output.
        if (!empty($this->rendered['blocks'][$uuid])) {
          $region_blocks[$uuid] = $this->rendered['blocks'][$uuid];
        }
      }
      $this->rendered['regions'][$region_id] = $this->renderRegion($region_id, $region_blocks);
    }

    return $this->rendered['regions'];
  }

  /**
   * Overrides LayoutRendererStandard::renderRegion().
   *
   * @return string
   *   The rendered, HTML string output of the passed-in region.
   */
  public function renderRegion($region_id, $blocks) {
    $style_settings = $this->prepared['regions'][$region_id]['style settings'];
    $tag = isset($style_settings['element']) ? $style_settings['element'] : '';
    $classes = array();
    if (isset($style_settings['classes'])) {
      $classes = explode(' ', $style_settings['classes']);
      foreach ($classes as $n => $class) {
        $classes[$n] = backdrop_clean_css_identifier($class, array());
      }
      $classes = array_filter($classes);
    }

    return theme('layout_region_inner', array('blocks' => $blocks, 'tag' => $tag, 'classes' => $classes));
  }

  /**
   * Overrides LayoutRendererStandard::renderBlocks().
   *
   * @return array
   *   The array of rendered blocks, keyed on block UUID.
   */
  public function renderBlocks() {
    $this->rendered['blocks'] = array();

    foreach ($this->prepared['blocks'] as $uuid => $block) {
      if ($block->status && $content = $this->renderBlock($block)) {
        $this->rendered['blocks'][$uuid] = $content;
      }
    }

    return $this->rendered['blocks'];
  }

  /**
   * Overrides LayoutRendererStandard::renderBlock().
   *
   * @param Block $block
   *   The block to be rendered.
   *
   * @return string
   *   The rendered block as HTML.
   */
  public function renderBlock($block) {
    $output = '';
    if ($content = $block->getContent()) {
      $style = layout_get_style_info($block->style->plugin);

      // Ensure block content is a renderable array for contextual links.
      if (is_string($content)) {
        $content = array(
          '#markup' => $content,
        );
      }
      // Add the contextual links.
      if (!($block->module === 'system' && $block->delta === 'main')) {
        $content['#contextual_links']['layout'] = array('admin/structure/layouts/manage/' . $this->layout->name . '/modal', array($block->uuid));
      }

      $data = array(
        'content' => $content,
      );

      // Assemble a structured content array.
      $content = (object) array(
        'content' => $data['content'],
      );

      if (isset($style) && isset($style['block theme'])) {
        $output = theme($style['block theme'] . '__' . $block->module . '__' . strtr($block->delta, '-', '_'), array('content' => $content, 'layout' => $this->layout, 'block' => $block, 'style' => $style, 'settings' => $block->style->settings));
      }
      else {
        // Fallback.
        $output = theme('block', array('content' => $content, 'layout' => $this->layout, 'block' => $block));
      }
    }
    return $output;
  }

}
