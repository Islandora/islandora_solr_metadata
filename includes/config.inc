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
function islandora_solr_metadata_config_form($form, &$form_state, $configuration_id) {
  module_load_include('inc', 'islandora', 'includes/content_model.autocomplete');
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $field_to_add = FALSE;
  $cmodel_to_add = FALSE;
  $data = array();

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
          unset($data[$key]);
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

    if ($form_state['triggering_element']['#value'] == t('Configure')) {
      // Triggered field #.
      $field = $form_state['triggering_element']['#field'];
      // Get field values to be passed off to the modal.
      $values = $form_state['solr_field_settings'][$field];
      // Prepare dialog values.
      $dialog_id = 'edit-islandora-solr-metadata-fields-terms-' . $field . '-dialog-button';
      $form_state['dialog'] = array(
        'dialog_id' => $dialog_id,
        'values' => $values,
      );
    }

    if ($form_state['triggering_element']['#value'] == t('Dialog')) {
      // Triggered field #.
      $field = $form_state['triggering_element']['#field'];
      // Get dialog values from recent dialog submit.
      $dialog_values = $form_state['input']['_dialog_values'] = json_decode($form_state['input']['_dialog_values'], TRUE);
      $structured_data = array(
        'schema_location' => $dialog_values['schema_location'],
        'namespace' => $dialog_values['namespace'],
        'value' => $dialog_values['value'],
      );
      // Save the dialog values in the form state.
      $form_state['solr_field_settings'][$field] = array(
        'solr_field' => $form_state['solr_field_settings'][$field]['solr_field'],
        'display_label' => $dialog_values['display_label'],
        'hyperlink' => empty($dialog_values['hyperlink']) ? 0 : 1,
        'date_format' => empty($dialog_values['date_format']) ? NULL : $dialog_values['date_format'],
        'permissions' => empty($dialog_values['enable_permissions']) ? 0 : $dialog_values['permissions'],
        'structured_data' => $structured_data,
      );
    }
  }
  $form = array(
    '#tree' => TRUE,
    '#attached' => array(
      'library' => array(
        array(
          'system', 'ui.dialog',
        ),
      ),
      'js' => array(
        drupal_get_path('module', 'islandora_solr_metadata') . '/js/islandora_solr_metadata.admin.js',
      ),
      'css' => array(
        drupal_get_path('module', 'islandora_solr_metadata') . '/css/islandora_solr_metadata.admin.css',
      ),
    ),
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

  if ($field_to_add !== FALSE) {
    $data[] = $field_to_add;
  }

  $form['islandora_solr_metadata_fields']['table_wrapper'] = array(
    '#prefix' => '<div id="islandora-solr-metadata-fields-wrapper">',
    '#suffix' => '</div>',
  );
  $form['islandora_solr_metadata_fields']['table_wrapper']['table'] = islandora_solr_metadata_management($data);
  // The content of the popup dialog.
  $form['islandora_solr_metadata_fields']['islandora_solr_metadata_admin_dialog'] = array(
    '#theme_wrappers' => array('container'),
    '#id' => 'islandora-solr-metadata-admin-dialog',
    '#weight' => 50,
  );
  $form['islandora_solr_metadata_fields']['islandora_solr_metadata_admin_dialog']['title'] = array(
    '#markup' => '<h2 id="islandora-solr-metadata-admin-dialog-title"></h2>',
  );
  $form['islandora_solr_metadata_fields']['islandora_solr_metadata_admin_dialog']['body'] = array(
    '#theme_wrappers' => array('container'),
    '#id' => 'islandora-solr-metadata-admin-dialog-body',
    '#markup' => t('Default dialog text'),
  );
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
  $field_settings = isset($form_state['solr_field_settings']) ? $form_state['solr_field_settings'] : array();

  // Loop through all of the values from the form state and database to
  // construct fields to be used within the modal.
  foreach ($data as $values) {
    if (!isset($field_settings[$values['weight']])) {
      $field_settings[$values['weight']] = array(
        'solr_field' => $values['solr_field'],
        'display_label' => isset($values['display_label']) ? $values['display_label'] : '',
        'hyperlink' => isset($values['hyperlink']) ? $values['hyperlink'] : 0,
        'date_format' => isset($values['date_format']) ? $values['date_format'] : NULL,
        'permissions' => isset($values['permissions']) ? $values['permissions'] : array(),
        'structured_data' => isset($values['structured_data']) ? $values['structured_data'] : NULL,
      );
    }
  }
  $form_state['solr_field_settings'] = $field_settings;
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
    // database. At the same time let's grab the values in the form state!
    $fields_fs_mapped = array();
    foreach ($fields_fs as $field) {
      $fields_fs_mapped[$field['solr_field']] = $field;
      foreach ($form_state['solr_field_settings'] as $values) {
        foreach ($values as $key => $val) {
          $fields_fs_mapped[$values['solr_field']][$key] = $val;
        }
      }
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
    $map = function($field) use($delta) {
      $value = $field['weight'];
      return array(
        'remove_field' => array(
          '#type' => 'checkbox',
          '#default_value' => isset($field['remove']) ? $field['remove'] : FALSE,
        ),
        'solr_field_markup' => array('#markup' => filter_xss($field['solr_field'])),
        'configure' => array(
          '#type' => 'link',
          '#title' => t('configure'),
          '#href' => '#',
          '#options' => array(
            'attributes' => array(
              'class' => array(
                'islandora-solr-metadata-configure-link',
              ),
            ),
          ),
        ),
        'configure_button' => array(
          '#type' => 'submit',
          '#value' => t('Configure'),
          '#field' => $value,
          '#attributes' => array(
            'class' => array(
              'islandora-solr-metadata-configure-submit',
            ),
          ),
          '#submit' => array('islandora_solr_metadata_update_fields_submit'),
          '#name' => 'display_fields_configure_' . $value,
          '#id' => 'edit-islandora-solr-metadata-fields-terms-' . $value . '-configure-button',
          '#ajax' => array(
            'callback' => 'islandora_solr_metadata_admin_settings_field_configure',
            'wrapper' => 'islandora-solr-metadata-fields-wrapper',
            'effect' => 'none',
            'event' => 'click',
            'progress' => array('message' => '', 'type' => 'throbber'),
          ),
        ),
        'dialog_button' => array(
          '#type' => 'submit',
          '#value' => t('Dialog'),
          '#field' => $value,
          '#attributes' => array('class' => array('islandora-solr-metadata-dialog-submit')),
          '#submit' => array('islandora_solr_metadata_update_fields_submit'),
          '#name' => 'display_fields_dialog_' . $value,
          '#id' => 'edit-islandora-solr-metadata-fields-terms-' . $value . '-dialog-button',
          '#ajax' => array(
            'callback' => '_islandora_solr_update_fields',
            'wrapper' => 'islandora-solr-metadata-fields-wrapper',
            'effect' => 'none',
            'event' => 'click',
            'progress' => array('message' => '', 'type' => 'throbber'),
          ),
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
      array('data' => t('Weight'), 'class' => 'islandora-solr-metadata-field-weight'),
      array('data' => t('Operations'), 'colspan' => 4),
    ),
    'rows' => $rows,
    '#theme' => 'islandora_solr_metadata_management_table',
    '#empty' => t('No fields associated'),
  );
  return $form;
}

/**
 * Submit callback function for fields.
 *
 * Used by 'Configure' and 'Dialog'.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 */
function islandora_solr_metadata_update_fields_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX callback function for 'Configure' link/button.
 *
 * Passes values from the field, populates and triggers the modal dialog.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 *
 * @return array
 *   An array of AJAX commands.
 */
function islandora_solr_metadata_admin_settings_field_configure($form, &$form_state) {
  $variables = $form_state['dialog'];

  $commands = array();
  // Open dialog.
  $commands[] = ajax_command_invoke('#islandora-solr-metadata-admin-dialog', 'dialog', array('open'));
  // Dialog title.
  $title = t('Configure field: @field', array(
    '@field' => $variables['values']['solr_field'],
  ));
  $commands[] = ajax_command_invoke('#islandora-solr-metadata-admin-dialog-title', 'html', array($title));
  // Dialog body.
  $form = drupal_get_form('islandora_solr_metadata_configure_field_form', $variables);
  $body = drupal_render($form);
  $commands[] = ajax_command_invoke('#islandora-solr-metadata-admin-dialog-body', 'html', array($body));
  // Attach behaviors (re-attaches javascript).
  $commands[] = ajax_command_invoke(NULL, 'islandoraSolrMetadataAttachBehaviors', array());

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * AJAX callback function for the modal dialog submit button.
 *
 * Closes the modal dialog and passes the values to the main form.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 *
 * @return array
 *   An array of AJAX commands.
 */
function islandora_solr_metadata_admin_settings_dialog($form, &$form_state) {
  $dialog_id = $form_state['dialog']['dialog_id'];

  $commands = array();
  // Close dialog.
  $commands[] = ajax_command_invoke('#islandora-solr-metadata-admin-dialog', 'dialog', array('close'));

  // Call function which attaches the dialog form values to
  // Drupal.ajax.{dialog-button-id}.options.data.
  // After triggering the dialog ajax callback, the dialog values will be added
  // to the $form_state of the main form.
  $data = array(
    'id' => $dialog_id,
    'values' => $form_state['input'],
  );
  $commands[] = ajax_command_invoke(NULL, 'islandoraSolrMetadataDialogValues', array($data));
  // Trigger dialog button.
  $commands[] = ajax_command_invoke('#' . $dialog_id, 'trigger', array('click'));

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Constructs the field configuration form for display within a modal.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param array $variables
 *   An array of variables to be passed off to the modal containing:
 *   -solr_field: A string containing the Solr field name.
 *   -dialog_id: The ID of the dialog value in the form.
 *   -values: Default values to be used in the modal which is an array
 *   containing:
 *     -display_label: A string containing the display label for the field.
 *     -hyperlink: A bool denoting whether or not the field is to link to Solr
 *     search results.
 *     -date_format: A string denoting a date format to be passed to date() or
 *     NULL if one has not been defined or the field is not a date.
 *     -permissions: An array of arrays representing permissions for the field
 *     where each array contains the Drupal role id as the key and a bool
 *     denoting viewable or not as the value.
 *
 * @return array
 *   The array to be rendered within the modal.
 */
function islandora_solr_metadata_configure_field_form($form, &$form_state, $variables) {
  module_load_include('inc', 'islandora_solr', 'includes/luke');
  form_load_include($form_state, 'inc', 'islandora_solr_metadata', 'includes/config');

  $form_state['dialog'] = $variables;
  $field = $variables['values'];
  $solr_field = $variables['values']['solr_field'];
  $form['options'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('scroll')),
    '#id' => 'islandora-solr-metadata-admin-dialog-form',
  );
  $form['options']['display_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Display Label'),
    '#default_value' => isset($field['display_label']) ? filter_xss($field['display_label']) : '',
    '#weight' => -1,
  );
  $form['options']['hyperlink'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hyperlink'),
    '#description' => t('Link this field to Solr search results.'),
    '#default_value' => isset($field['hyperlink']) ? $field['hyperlink'] : 0,
    '#weight' => 0,
  );
  $form['options']['structured_data'] = array(
    '#type' => 'fieldset',
    '#title' => t('Structured Data'),
    '#description' => t('Note: If this feature is used, all fields must be filled out; otherwise, any value entered will be ignored to avoid forming incomplete structured data tags.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 2,
  );
  $form['options']['structured_data']['schema_location'] = array(
    '#type' => 'textfield',
    '#title' => t('Schema Location'),
    '#default_value' => isset($field['structured_data']['schema_location']) ? $field['structured_data']['schema_location'] : '',
    '#description' => t('The location of the schema. For example, http://purl.org/dc/terms/ for dcterms.'),
  );
  $form['options']['structured_data']['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace'),
    '#default_value' => isset($field['structured_data']['namespace']) ? $field['structured_data']['namespace'] : '',
    '#description' => t('The namespace for structured data. For example, dcterms for dcterms.'),
  );
  $form['options']['structured_data']['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#default_value' => isset($field['structured_data']['value']) ? $field['structured_data']['value'] : '',
    '#description' => t('The property value for the structured data. For example, title for dcterms:title.'),
  );
  $values = array(
    'permissions' => $field['permissions'],
  );
  $luke_result = islandora_solr_get_luke(NULL, $solr_field);
  if (isset($luke_result['fields'][$solr_field]['type']) && $luke_result['fields'][$solr_field]['type'] == 'date') {
    $form['options']['date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Date format'),
      '#default_value' => isset($field['date_format']) ? $field['date_format'] : '',
      '#description' => t('The format of the date, as it will be displayed in the search results. Use <a href="!url" target="_blank">PHP date()</a> formatting.', array('!url' => 'http://php.net/manual/function.date.php')),
      '#weight' => 0,
    );
  }
  islandora_solr_metadata_append_permissions_and_actions($values, $form);

  return $form;
}

/**
 * Utility function to append permissions and actions to the modal.
 *
 * @param array $values
 *   An array of values.
 * @param array $form
 *   An array representing the Drupal form, passed by reference.
 *
 * @return array
 *   An array containing the permissions and actions to be rendered.
 */
function islandora_solr_metadata_append_permissions_and_actions($values, &$form) {
  module_load_include('inc', 'islandora_solr', 'includes/admin');
  $permissions = isset($values['permissions']) ? $values['permissions'] : array();

  // Let's re-use some Solr's code cause we can.
  $permissions_disable = _islandora_solr_permissions_disable();
  $permissions_default = _islandora_solr_permissions_default();
  $form['options']['permissions_fieldset'] = islandora_solr_get_admin_permissions_fieldset($permissions, $permissions_default, $permissions_disable, FALSE);

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-buttons')),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Apply'),
    '#weight' => 5,
    '#field' => 'dialog_submit',
    '#name' => 'result-fields-dialog-submit',
    '#submit' => array('islandora_solr_metadata_update_fields_submit'),
    '#ajax' => array(
      'callback' => 'islandora_solr_metadata_admin_settings_dialog',
      'wrapper' => 'islandora-solr-metadata-admin-dialog-form',
      'effect' => 'none',
      'event' => 'click',
      'progress' => array('message' => '', 'type' => 'throbber'),
    ),
  );
  return $form;
}
