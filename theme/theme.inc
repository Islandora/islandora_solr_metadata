<?php
/**
 * @file
 * Theme functions for the islandora_solr_metadata module.
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

/**
 * Implements hook_preprocess().
 */
function islandora_solr_metadata_preprocess_islandora_solr_metadata_display(array &$variables) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $object = $variables['islandora_object'];
  $db_fields = array();
  $solr_fields = array();
  $render_metadata = array();
  $associations = islandora_solr_metadata_get_associations_by_cmodels($object->models);

  foreach ($associations as $configuration_id) {
    $field = islandora_solr_metadata_get_fields($configuration_id['configuration_id']);
    $db_fields = array_merge($solr_fields, $field);
  }
  if (count($db_fields)) {
    uasort($db_fields, 'drupal_sort_weight');
    foreach ($db_fields as $key => $value) {
      if (!in_array($value, $solr_fields)) {
        $solr_fields[] = $value['solr_field'];
        // Make an array for use later on.
        $render_metadata[$key] = array(
          'display_label' => $value['display_label'],
          'value' => array(),
        );
      }
    }
    $query_processor = new IslandoraSolrQueryProcessor();
    $query_processor->solrQuery = 'PID:"' . $object->id . '"';
    $query_processor->solrParams['fl'] = 'PID, ' . implode(',', $solr_fields);
    $query_processor->executeQuery(FALSE);
    if ($query_processor->islandoraSolrResult['response']['numFound'] > 0) {
      $solr_results_doc = $query_processor->islandoraSolrResult['response']['objects']['0']['solr_doc'];
      foreach ($solr_results_doc as $solr_field => $value) {
        // Only display fields that were present in our original list.
        if (in_array($solr_field, $solr_fields)) {
          // Want the values in array form even if it's a single valued field.
          if (!is_array($value)) {
            $value = (array) $value;
          }
          $render_metadata[$solr_field]['value'] = array_filter(array_map('trim', $value));
        }
      }
    }
  }
  else {
    drupal_set_message(t('Unable to find @pid object in the Solr index. Contact the administrator!', array('@pid' => $object->id)), 'error');
  }
  $variables['solr_fields'] = $render_metadata;
  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($object);
}

/**
 * Implements hook_preprocess().
 */
function islandora_solr_metadata_preprocess_islandora_solr_metadata_description(array &$variables) {
  $object = $variables['islandora_object'];
  $description_render = array();
  foreach ($variables['description'] as $description) {
    $query_processor = new IslandoraSolrQueryProcessor();
    $query_processor->solrQuery = 'PID:"' . $object->id . '"';
    $query_processor->solrParams['fl'] = "PID, {$description['description_field']}";
    $query_processor->executeQuery(FALSE);
    if (count($query_processor->islandoraSolrResult['response']['numFound']) > 0) {
      $solr_results_doc = $query_processor->islandoraSolrResult['response']['objects']['0']['solr_doc'];
      foreach ($solr_results_doc as $solr_field => $value) {
        if ($solr_field == $description['description_field']) {
          // Want the values in array form even if it's a single valued field.
          if (!is_array($value)) {
            $value = (array) $value;
          }
          $description_render[$solr_field]['display_label'] = $description['description_label'];
          $description_render[$solr_field]['value'] = array_filter(array_map('trim', $value));
        }
      }
    }
  }
  $variables['description'] = $description_render;
}
