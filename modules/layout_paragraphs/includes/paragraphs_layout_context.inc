<?php
/**
 * @file
 * Layout context handler for entities.
 */

/**
 * Layout context for Paragraphs.
 */
class ParagraphsLayoutContext extends LayoutContext {
  public $usageType = LayoutContext::USAGE_TYPE_CUSTOM;

  /**
   * Return the indicator for this context.
   */
  public function type() {
    return 'paragraphs';
  }

  /**
   * Assemble a human-readable label of this object.
   */
  public function label() {
    return t('Paragraphs');
  }
}
