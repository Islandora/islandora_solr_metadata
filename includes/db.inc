<?php

/**
 * @file
 * This file contains DB query functions
 */

function islandora_solr_metadata_get_associations() {
  $associations = array();
  $result = db_query('SELECT configuration_id, configuration_name FROM {islandora_solr_metadata_associations}');
  foreach ($result as $record) {
    $associations[] = array(
      'name' => $record->configuration_name,
      'id' => $record->configuration_id,
    );
  }
  return $associations;
}

function islandora_solr_metadata_get_cmodels($configuration_id) {
  $result = db_query('SELECT cmodel FROM {islandora_solr_metadata_cmodels} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  return $result->fetchAllAssoc('cmodel', PDO::FETCH_ASSOC);
}

function islandora_solr_metadata_get_all_associated_cmodels() {
  $result = db_query ('SELECT DISTINCT cmodel FROM {islandora_solr_metadata_cmodels}');
  return $result->fetchCol();
}

function islandora_solr_metadata_add_configuration($configuration_name) {
  db_insert('islandora_solr_metadata_associations')->
    fields(
      array(
        'configuration_name' => $configuration_name,
      )
    )
    ->execute();
}

function islandora_solr_metadata_get_fields($configuration_id) {
  $result = db_query('SELECT * FROM {islandora_solr_metadata_fields} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  return $result->fetchAllAssoc('solr_field', PDO::FETCH_ASSOC);
}

function islandora_solr_metadata_retrieve_configuration_name($configuration_id) {
  $result = db_query('SELECT configuration_name from {islandora_solr_metadata_associations} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  return $result->fetchColumn();
}

function islandora_solr_metadata_delete_association($configuration_id) {

}

function islandora_solr_metadata_delete_fields($configuration_id) {
  dsm($configuration_id, 'config id');
  db_delete('islandora_solr_metadata_fields')->condition('configuration_id', $configuration_id)->execute();
}

function islandora_solr_metadata_add_field($row) {
  $fields = array(
    'configuration_id' => $row['configuration_id'],
    'solr_field' => $row['solr_field'],
    'display_label' => $row['display_label'],
    'weight' => $row['weight'],
  );
  db_insert('islandora_solr_metadata_fields')->fields($fields)->execute();
}
