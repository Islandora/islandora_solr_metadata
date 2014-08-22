<?php
/**
 * @file
 * Houses the form and utilities used in islandora_solr_metadata configurations.
 */

/**
 * The configuration form for the islandora_solr_metadata module.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param int $configuration_id
 *   The ID of the configuration we are currently editing.
 *
 * @return array
 *   An array representing the configuration form.
 */
function islandora_solr_metadata_config_form($form, $form_state, $configuration_id) {
  module_load_include('inc', 'islandora', 'includes/content_model.autocomplete');
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $field_to_add = FALSE;
  $cmodel_to_add = FALSE;
  $data = array();

  // AJAX callback handling.
  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-add-field') {
      $field_to_add = array(
        'solr_field' => filter_xss($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields']),
        // Arbitrary large sort weight so it always comes last.
        'weight' => 10000,
      );
    }

    if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-fields-remove-selected') {
      $rows_to_remove = array();
      foreach ($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'] as $key => $row) {
        if ($row['remove_field'] == TRUE) {
          $rows_to_remove[] = $key;
        }
      }
      foreach ($rows_to_remove as $row_number) {
        unset($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'][$row_number]);
      }
    }

    if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-cmodels-add-cmodel') {
      $cmodel_to_add = array('cmodel' => $form_state['values']['islandora_solr_metadata_cmodels']['table_wrapper']['cmodel_options']['cmodel_select']);
    }

    if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-cmodels-remove-selected') {
      foreach ($form_state['values']['islandora_solr_metadata_cmodels']['table_wrapper']['table'] as $key => $row) {
        if ($row !== 0) {
          unset($form_state['complete form']['islandora_solr_metadata_cmodels']['table_wrapper']['table']['#options'][$key]);
        }
      }
    }
  }
  $form = array(
    '#tree' => TRUE,
  );
  $form['islandora_solr_metadata_configuration_id'] = array(
    '#type' => 'value',
    '#value' => $configuration_id,
  );

  $form['islandora_solr_metadata_cmodels'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content Models'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  $form['islandora_solr_metadata_cmodels']['table_wrapper'] = array(
    '#prefix' => '<div id="islandora-solr-metadata-cmodels-wrapper">',
    '#suffix' => '</div>',
  );
  // If there are values in the form_state use them for persistence in case of
  // AJAX callbacks, otherwise grab fresh values from the database.
  if (isset($form_state['values'])) {
    if (isset($form_state['values']['islandora_solr_metadata_cmodels']['table_wrapper']['table'])) {
      $cmodels_associated = $form_state['complete form']['islandora_solr_metadata_cmodels']['table_wrapper']['table']['#options'];
    }
  }
  else {
    $cmodels_associated = islandora_solr_metadata_get_cmodels($configuration_id);
  }

  if ($cmodel_to_add !== FALSE) {
    $cmodels_associated[$cmodel_to_add['cmodel']] = $cmodel_to_add;
  }

  $form['islandora_solr_metadata_cmodels']['table_wrapper']['table'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'cmodel' => array('data' => t('Content Model Name')),
    ),
    '#options' => $cmodels_associated,
    '#empty' => t('No content models associated.'),
  );

  if (count($cmodels_associated)) {
    $form['islandora_solr_metadata_cmodels']['table_wrapper']['remove_selected'] = array(
      '#type' => 'button',
      '#value' => t('Remove selected'),
      '#name' => 'islandora-solr-metadata-cmodels-remove-selected',
      '#ajax' => array(
        'callback' => 'islandora_solr_metadata_cmodels_ajax',
        'wrapper' => 'islandora-solr-metadata-cmodels-wrapper',
      ),
    );
  }

  // Retrieve all content models and unset those currently in use in this
  // configuration and any others from other configurations.
  $add_options = islandora_get_content_model_names();
  foreach ($cmodels_associated as $entry) {
    unset($add_options[$entry['cmodel']]);
  }

  if (!empty($add_options)) {
    $form['islandora_solr_metadata_cmodels']['table_wrapper']['cmodel_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add content model'),
      '#collapsed' => TRUE,
      '#collapsible' => FALSE,
    );

    $form['islandora_solr_metadata_cmodels']['table_wrapper']['cmodel_options']['cmodel_select'] = array(
      '#type' => 'select',
      '#options' => $add_options,
    );
    $form['islandora_solr_metadata_cmodels']['table_wrapper']['cmodel_options']['cmodel_add'] = array(
      '#type' => 'button',
      '#value' => t('Add'),
      '#name' => 'islandora-solr-metadata-cmodels-add-cmodel',
      '#ajax' => array(
        'callback' => 'islandora_solr_metadata_cmodels_ajax',
        'wrapper' => 'islandora-solr-metadata-cmodels-wrapper',
      ),
    );
  }

  $form['islandora_solr_metadata_fields'] = array(
    '#type' => 'fieldset',
    '#title' => 'Display fields',
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  // If there are values in the form_state use them for persistence in case of
  // AJAX callbacks, otherwise grab fresh values from the database.
  if (isset($form_state['values'])) {
    if (isset($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table'])) {
      $data = $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'];
    }
  }
  else {
    $data = islandora_solr_metadata_get_fields($configuration_id, FALSE);
  }

  if ($field_to_add !== FALSE) {
    $data[] = $field_to_add;
  }

  $form['islandora_solr_metadata_fields']['table_wrapper'] = array(
    '#prefix' => '<div id="islandora-solr-metadata-fields-wrapper">',
    '#suffix' => '</div>',
  );
  $form['islandora_solr_metadata_fields']['table_wrapper']['table'] = islandora_solr_metadata_management($data);
  if (count($form['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'])) {
    $form['islandora_solr_metadata_fields']['table_wrapper']['remove_selected'] = array(
      '#type' => 'button',
      '#value' => t('Remove selected'),
      '#name' => 'islandora-solr-metadata-fields-remove-selected',
      '#ajax' => array(
        'callback' => 'islandora_solr_metadata_fields_ajax',
        'wrapper' => 'islandora-solr-metadata-fields-wrapper',
      ),
    );
  }
  $form['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add field'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  $form['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields'] = array(
    '#type' => 'textfield',
    '#description' => t('A field from within Solr'),
    '#size' => 105,
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    '#default_value' => '',
  );
  $form['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['add_field'] = array(
    '#type' => 'button',
    '#value' => t('Add'),
    '#name' => 'islandora-solr-metadata-add-field',
    '#ajax' => array(
      'callback' => 'islandora_solr_metadata_fields_ajax',
      'wrapper' => 'islandora-solr-metadata-fields-wrapper',
    ),
  );
  $form['islandora_solr_metadata_fields']['description_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Description'),
    '#description' => t("The field used for rendering the description beneath the object's content"),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );

  $description = islandora_solr_metadata_retrieve_description($configuration_id, FALSE);
  $form['islandora_solr_metadata_fields']['description_fieldset']['available_solr_fields'] = array(
    '#type' => 'textfield',
    '#description' => t('A field from within Solr'),
    '#size' => 105,
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    '#default_value' => $description['description_field'],
  );
  $form['islandora_solr_metadata_fields']['description_fieldset']['display_label'] = array(
    '#type' => 'textfield',
    '#description' => t('A label for displaying'),
    '#size' => 45,
    '#default_value' => $description['description_label'],
    '#states' => array(
      'visible' => array(
        ':input[name="islandora_solr_metadata_fields[description_fieldset][available_solr_fields]"]' => array('empty' => FALSE),
      ),
    ),
  );
  $form['islandora_solr_metadata_save'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 10,
  );
  $form['islandora_solr_metadata_delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete configuration'),
    '#weight' => 10,
  );
  return $form;
}

/**
 * Validation for the islandora_solr_metadata configuration form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_solr_metadata_config_form_validate($form, $form_state) {
  if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-add-field') {
    module_load_include('inc', 'islandora_solr_search', 'includes/luke');
    $solr_luke = islandora_solr_get_luke();
    $luke_fields = array_keys($solr_luke['fields']);

    if (!in_array($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields'], $luke_fields)) {
      form_set_error('islandora_solr_metadata_fields][table_wrapper][add_fieldset][available_solr_fields', t('The field @field is not a valid field within Solr!', array('@field' => $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields'])));
    }
    else {
      $added_values = array();
      if (isset($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table'])) {
        foreach ($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'] as $row) {
          $added_values[] = $row['solr_field'];
        }
      }
      if (in_array($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields'], $added_values)) {
        form_set_error('islandora_solr_metadata_fields][table_wrapper][add_fieldset][available_solr_fields', t('The field @field already exists in this configuration!', array('@field' => $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['add_fieldset']['available_solr_fields'])));
      }
    }
  }

  if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-fields-remove-selected') {
    $rows_to_remove = array();
    foreach ($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'] as $key => $row) {
      if ($row['remove_field'] == TRUE) {
        $rows_to_remove[] = $key;
      }
    }
    if (count($rows_to_remove) === 0) {
      form_set_error('islandora_solr_metadata', t('Must select at least one entry to remove!'));
    }
  }

  if ($form_state['triggering_element']['#name'] == 'islandora-solr-metadata-cmodels-remove-selected') {
    $rows_to_remove = array();
    foreach ($form_state['values']['islandora_solr_metadata_cmodels']['table_wrapper']['table'] as $key => $row) {
      if ($row !== 0) {
        $rows_to_remove[] = $key;
      }
    }
    if (count($rows_to_remove) === 0) {
      form_set_error('islandora_solr_metadata', t('Must select at least one entry to remove!'));
    }
  }

  if ($form_state['triggering_element']['#value'] == 'Save configuration') {
    $solr_field = $form_state['values']['islandora_solr_metadata_fields']['description_fieldset']['available_solr_fields'];
    if (!empty($solr_field)) {
      module_load_include('inc', 'islandora_solr_search', 'includes/luke');
      $solr_luke = islandora_solr_get_luke();
      $luke_fields = array_keys($solr_luke['fields']);
      if (!in_array($solr_field, $luke_fields)) {
        form_set_error('islandora_solr_metadata_fields][description_fieldset][available_solr_fields', t('The field @field is not a valid field within Solr!', array('@field' => $solr_field)));
      }
    }
  }
}

/**
 * Submit handler for the islandora_solr_metadata configuration form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_solr_metadata_config_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $configuration_id = $form_state['values']['islandora_solr_metadata_configuration_id'];

  if ($form_state['clicked_button']['#value'] == 'Save configuration') {
    // Grab existing entries first for comparison.
    $fields_db = islandora_solr_metadata_get_fields($configuration_id, FALSE);
    if (isset($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table'])) {
      $fields_fs = $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'];
    }
    else {
      $fields_fs = array();
    }

    // Need to map the fields so that the keys match what we get out of the
    // database.
    $fields_fs_mapped = array();
    foreach ($fields_fs as $field) {
      $fields_fs_mapped[$field['solr_field']] = $field;
    }

    $cmodels_db = islandora_solr_metadata_get_cmodels($configuration_id);
    $cmodels_fs = $form_state['complete form']['islandora_solr_metadata_cmodels']['table_wrapper']['table']['#options'];

    $cmodels_deletion = array_diff_key($cmodels_db, $cmodels_fs);
    $cmodels_insertion = array_diff_key($cmodels_fs, $cmodels_db);

    // Handle insertion and deletion of content models first.
    if (count($cmodels_insertion)) {
      islandora_solr_metadata_add_content_models($configuration_id, $cmodels_insertion);
    }
    if (count($cmodels_deletion)) {
      islandora_solr_metadata_delete_content_models($configuration_id, $cmodels_deletion);
    }

    $fields_deletion = array_diff_key($fields_db, $fields_fs_mapped);
    $fields_insertion = array_diff_key($fields_fs_mapped, $fields_db);

    if (count($fields_insertion)) {
      islandora_solr_metadata_add_fields($configuration_id, $fields_insertion);
    }
    if (count($fields_deletion)) {
      islandora_solr_metadata_delete_fields($configuration_id, $fields_deletion);
    }

    $fields_update = array_intersect_key($fields_fs_mapped, $fields_db);
    if (count($fields_update)) {
      islandora_solr_metadata_update_fields($configuration_id, $fields_update);
    }

    $description_field = $form_state['values']['islandora_solr_metadata_fields']['description_fieldset']['available_solr_fields'];
    $description_label = $form_state['values']['islandora_solr_metadata_fields']['description_fieldset']['display_label'];
    islandora_solr_metadata_update_description($configuration_id, $description_field, $description_label);
    drupal_set_message(t('The Solr metadata display configuration options have been saved.'));
  }

  if ($form_state['clicked_button']['#value'] == 'Delete configuration') {
    drupal_goto("admin/islandora/search/islandora_solr_metadata/config/delete/$configuration_id");
  }
}

/**
 * AJAX callback for the fields portion of the configuration form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array representing the Drupal form state.
 *
 * @return array
 *   An array representing the portion of the form we are rendering.
 */
function islandora_solr_metadata_fields_ajax(&$form, &$form_state) {
  return $form['islandora_solr_metadata_fields']['table_wrapper'];
}

/**
 * AJAX callback for the content models portion of the configuration form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array representing the Drupal form state.
 *
 * @return array
 *   An array representing the portion of the form we are rendering.
 */
function islandora_solr_metadata_cmodels_ajax(&$form, &$form_state) {
  return $form['islandora_solr_metadata_cmodels']['table_wrapper'];
}

/**
 * Helper function to construct the fields tableselect.
 *
 * @param array $data
 *   An array of data representing rows to be rendered in the tableselect.
 *
 * @return array
 *   An array representing the tableselect to be rendered.
 */
function islandora_solr_metadata_management($data) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $form = array();
  $rows = array();

  if ($data) {
    uasort($data, 'drupal_sort_weight');
    $delta = count($data);
    $weight = 1;
    $map = function($field) use($delta, &$weight) {
      $value = $weight;
      $weight++;
      return array(
        'remove_field' => array(
          '#type' => 'checkbox',
          '#default_value' => isset($field['remove']) ? $field['remove'] : FALSE,
        ),
        'solr_field_markup' => array('#markup' => filter_xss($field['solr_field'])),
        'display_label' => array(
          '#type' => 'textfield',
          '#default_value' => isset($field['display_label']) ? filter_xss($field['display_label']) : '',
        ),
        'hyperlink' => array(
          '#type' => 'checkbox',
          '#default_value' => isset($field['hyperlink']) ? $field['hyperlink'] : FALSE,
        ),
        'weight' => array(
          '#type' => 'weight',
          '#title' => t('Weight'),
          '#default_value' => $value,
          '#delta' => $delta,
        ),
        'solr_field' => array(
          '#type' => 'value',
          '#value' => filter_xss($field['solr_field']),
        ),
      );
    };
    $rows = array_map($map, $data);
  }
  // Add the table to the form.
  $form['#tree'] = TRUE;
  $form['table'] = array(
    '#type' => 'markup',
    '#header' => array(
      t('Remove'),
      t('Solr Field'),
      t('Display Label'),
      t('Hyperlink'),
      '',
    ),
    'rows' => $rows,
    '#theme' => 'islandora_solr_metadata_management_table',
    '#empty' => t('No fields associated'),
  );
  return $form;
}
