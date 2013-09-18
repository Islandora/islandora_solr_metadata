<?php

/**
 * @file
 * theme/theme.inc
 *
 * Theme functions.
 */

/**
 * Implements theme_element().
 */
function theme_islandora_solr_metadata_management_table(array $variables) {
  $table = $variables['table'];
  $rows = array();
  $weight_class = 'islandora-solr-metadata-weight';
  foreach (element_children($table['rows']) as $id) {
    $row = &$table['rows'][$id];
    $row['weight']['#attributes'] = array('class' => array($weight_class));
    $rows[] = array(
      'data' => array(
        drupal_render($row['remove_field']),
        drupal_render($row['solr_field_markup']),
        drupal_render($row['display_label']),
        drupal_render($row['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $table_id = 'entry-order';
  $output = theme('table', array(
    'header' => $table['#header'],
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
    'empty' => t('There are no entries available.')));
  $output .= drupal_render_children($table);
  drupal_add_tabledrag($table_id, 'order', 'sibling', $weight_class);
  return $output;
}