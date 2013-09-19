<?php

/**
 * @file
 * This file contains DB query functions
 */

/**
 * Build up the key for a configuration name.
 *
 * @param int $config_id
 *   The ID of the configuration of which the name is to be translated.
 *
 * @return string
 *   A colon-separated string as used by the
 *   i18n_string_{update,remove,translate} functions.
 */
function islandora_solr_metadata_config_i18n_id($config_id) {
  return format_string('islandora_solr_metadata:configuration_name:@id', array(
    '@id' => $config_id,
  ));
}

/**
 * Build up the key for a field.
 *
 * @param int $config_id
 *   The ID of the configuration with which the given field is associated.
 * @param string $field_name
 *   The field of which the label is to be translated.
 *
 * @return string
 *   A colon-separated string as used by the
 *   i18n_string_{update,remove,translate} functions.
 */
function islandora_solr_metadata_field_i18n_id($config_id, $field_name) {
  return format_string('islandora_solr_metadata:field_display_label:@id:@field', array(
    '@id' => $config_id,
    '@field' => $field_name,
  ));
}

/**
 * Get all configurations available.
 *
 * @return array
 *   An array of associative arrays, each of which contains:
 *   - name: The name of the given configuration, possibly translated via
 *     i18n_string.
 *   - id: The ID of the given configuration.
 */
function islandora_solr_metadata_get_associations() {
  $associations = array();
  $result = db_query('SELECT configuration_id, configuration_name FROM {islandora_solr_metadata_associations}');
  foreach ($result as $record) {
    $config_name = $record->configuration_name;
    if (module_exists('i18n_string')) {
      $i18n_id = islandora_solr_metadata_config_i18n_id($record->configuration_id);
      $config_name = i18n_string_translate($i18n_id, $config_name);
    }

    $associations[] = array(
      'name' => $record->configuration_name,
      'id' => $record->configuration_id,
    );
  }
  return $associations;
}

/**
 * Get models associated with the given configuration.
 *
 * @param int $configuration_id
 *   The ID of the configuration for which to grab the content models.
 *
 * @return array
 *   An associative array mapping content model PIDs to an associative array
 *   containing:
 *   - cmodel: The PID, again... Blame Jordan. :P
 */
function islandora_solr_metadata_get_cmodels($configuration_id) {
  $result = db_query('SELECT cmodel FROM {islandora_solr_metadata_cmodels} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  return $result->fetchAllAssoc('cmodel', PDO::FETCH_ASSOC);
}

/**
 * Get all models which have already been associated with a configuration.
 *
 * @return array
 *   An array of strings, representing content model PIDs.
 */
function islandora_solr_metadata_get_all_associated_cmodels() {
  $result = db_query ('SELECT DISTINCT cmodel FROM {islandora_solr_metadata_cmodels}');
  return $result->fetchCol();
}

/**
 * Create a new configuration.
 *
 * @param string $configuration_name
 *   The name of a configuration to add the the DB.
 *
 * @return int
 *   The ID with which the configuration was created in the database.
 */
function islandora_solr_metadata_add_configuration($configuration_name) {
  $id = db_insert('islandora_solr_metadata_associations')->
    fields(
      array(
        'configuration_name' => $configuration_name,
      )
    )
    ->execute();

  if (module_exists('i18n_string')) {
    $i18n_id = islandora_solr_metadata_config_i18n_id($id);
    i18n_string_update($i18n_id, $config_name);
  }

  return $id;
}

/**
 * Get all fields in the given display.
 *
 * @param int $configuration_id
 *   The ID for the configuration for which to get the list of fields.
 * @param bool $translate
 *   Whether or not to attempt to translate the "display labels" with i18n.
 *   Defaults to TRUE.
 *
 * @return array
 *   An associative array of associative arrays, representing Solr fields
 *   mapped to the details about the given field, including:
 *   - configuration_id
 *   - display_label
 *   - weight
 *   - solr_field
 */
function islandora_solr_metadata_get_fields($configuration_id, $translate = TRUE) {
  $result = db_query('SELECT * FROM {islandora_solr_metadata_fields} WHERE configuration_id = :configuration_id ORDER BY weight', array(':configuration_id' => $configuration_id));

  $fields = $result->fetchAllAssoc('solr_field', PDO::FETCH_ASSOC);

  if ($translate && module_exists('i18n_string')) {
    foreach ($fields as $field => &$field_info) {
      $i18n_id = islandora_solr_metadata_field_i18n_id($configuration_id, $field);
      $field_info['display_label'] = i18n_string_translate($i18n_id, $field_info['display_label']);
    }
    // XXX: foreach with references can be silly.
    unset($field_info);
  }

  return $fields;
}

/**
 * Get the name for the given configuration.
 *
 * @param int $configuration_id
 *   The ID for a configuration for which to grab the name.
 *
 * @return string
 *   The name of the configuration. Will be translated with i18n if the
 *   i18n_string module is enabled.
 */
function islandora_solr_metadata_retrieve_configuration_name($configuration_id) {
  $result = db_query('SELECT configuration_name from {islandora_solr_metadata_associations} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  $config_name = $result->fetchColumn();

  if (module_exists('i18n_string')) {
    $i18n_id = islandora_solr_metadata_config_i18n_id($configuration_id);
    $config_name = i18n_string_translate($i18n_id, $config_name);
  }

  return $config_name;
}

/**
 * Delete the given configuration indicated by the given ID.
 *
 * @param int $configuration_id
 *   The ID for a configuration to remove.
 */
function islandora_solr_metadata_delete_configuration($configuration_id) {
  if (module_exists('i18n_string')) {
    $fields = islandora_solr_metadata_get_fields($configuration_id, $translate);
    foreach (array_keys($fields) as $field) {
      $i18n_id = islandora_solr_metadata_field_i18n_id($configuration_id, $field);
      i18n_string_remove($i18n_id);
    }
  }
  db_delete('islandora_solr_metadata_fields')
    ->condition('configuration_id', $configuration_id)
    ->execute();

  db_delete('islandora_solr_metadata_cmodels')
    ->condition('configuration_id', $configuration_id)
    ->execute();

  if (module_exists('i18n_string')) {
    $i18n_id = islandora_solr_metadata_config_i18n_id($id);
    i18n_string_remove($i18n_id);
  }
  db_delete('islandora_solr_metadata_associations')
    ->condition('configuration_id', $configuration_id)
    ->execute();
}

/**
 * Check that a configuration exists with the given ID.
 *
 * @param int $configuration_id
 *   An ID to test.
 *
 * @return bool
 *   Whether or not the ID exists.
 */
function islandora_solr_metadata_configuration_exists($configuration_id) {
  $result = db_query('SELECT COUNT(*) FROM {islandora_solr_metadata_associations} WHERE configuration_id = :configuration_id', array(':configuration_id' => $configuration_id));
  return intval($result->fetchField()) > 0;
}

