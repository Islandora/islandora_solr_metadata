<?php

function islandora_solr_metadata_config_form($form, $form_state, $configuration_id) {
  module_load_include('inc', 'islandora', 'includes/content_model.autocomplete');
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $field_to_add = FALSE;
  $cmodel_to_add = FALSE;
  $data = array();
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
  // Form state + db shit.
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

  // Some check if anything is in the table...
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
  $form['islandora_solr_metadata_cmodels']['table_wrapper']['cmodel_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add content model'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  $add_options = islandora_get_content_model_names();
  foreach ($cmodels_associated as $entry) {
    unset($add_options[$entry['cmodel']]);
  }
  foreach (islandora_solr_metadata_get_all_associated_cmodels() as $cmodel) {
    unset($add_options[$cmodel]);
  }

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

  $form['islandora_solr_metadata_fields'] = array(
    '#type' => 'fieldset',
    '#title' => 'Display fields',
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  if (isset($form_state['values'])) {
    if (isset($form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table'])) {
      $data = $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'];
    }
  }
  else {
    $data = islandora_solr_metadata_get_fields($configuration_id);
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
    '#size' => 45,
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

}

function islandora_solr_metadata_config_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $configuration_id = $form_state['values']['islandora_solr_metadata_configuration_id'];
  
  if ($form_state['clicked_button']['#value'] == 'Save configuration') {
    // Grab existing entries first for comparison.
    $fields_db = islandora_solr_metadata_get_fields($configuration_id);
    if (isset($form_state['values']['islandora_solr_metadata_fields']['table'])) {
      $fields_fs = $form_state['values']['islandora_solr_metadata_fields']['table_wrapper']['table']['table']['rows'];
    }
    else {
      $fields_fs = array();
    }

    $cmodels_db = islandora_solr_metadata_get_cmodels($configuration_id);
    $cmodels_fs = $form_state['complete form']['islandora_solr_metadata_cmodels']['table_wrapper']['table']['#options'];

    $cmodels_deletion = array_diff_key($cmodels_db, $cmodels_fs);
    $cmodels_insertion = array_diff_key($cmodels_fs, $cmodels_db);

    // Handle insertion and deletion of content models first.
    if (count($cmodels_insertion)) {
    $query = db_insert('islandora_solr_metadata_cmodels')
      ->fields(array('configuration_id', 'cmodel'));
       foreach ($cmodels_insertion as $row) {
         $query->values(array(
           'configuration_id' => $configuration_id,
           'cmodel' => $row['cmodel'],
         ));
       }
      $query->execute();
    }
    if (count($cmodels_deletion)) {
      db_delete('islandora_solr_metadata_cmodels')
        ->condition('cmodel', array_keys($cmodels_deletion))
        ->condition('configuration_id', $configuration_id)
        ->execute();
    }

    $fields_deletion = array_diff_key($fields_db, $fields_fs);
    $fields_insertion = array_diff_key($fields_fs, $fields_db);
    if (count($fields_insertion)) {
      $query = db_insert('islandora_solr_metadata_fields')
        ->fields(array(
          'configuration_id',
          'solr_field',
          'display_label',
          'weight',
        )
      );
      foreach ($fields_insertion as $row) {
        $query->values(array(
          'configuration_id' => $configuration_id,
          'solr_field' => $row['solr_field'],
          'display_label' => $row['display_label'],
          'weight' => $row['weight'],
        ));
      }
      $query->execute();
    }
    if (count($fields_deletion)) {
      db_delete('islandora_solr_metadata_fields')
        ->condition('solr_field', array_keys($fields_deletion))
        ->condition('configuration_id', $configuration_id)
        ->execute();
    }



    $fields_update = array_intersect_key($fields_fs, $fields_db);
    foreach ($fields_update as $row) {
      db_update('islandora_solr_metadata_fields')
        ->fields(array(
          'weight' => $row['weight'],
          'display_label' => $row['display_label'],
        ))
        ->condition('solr_field', $row['solr_field'])
        ->condition('configuration_id', $configuration_id)
        ->execute();
    }
  }

  if ($form_state['clicked_button']['#value'] == 'Delete configuration') {
    drupal_goto("admin/islandora/search/islandora_solr_metadata/config/delete/$configuration_id");
  }
}

function islandora_solr_metadata_fields_ajax(&$form, &$form_state) {
  return $form['islandora_solr_metadata_fields']['table_wrapper'];
}

function islandora_solr_metadata_cmodels_ajax(&$form, &$form_state) {
  return $form['islandora_solr_metadata_cmodels']['table_wrapper'];
}

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
          '#default_value' => isset($field['remove']) ? $field['remove']: FALSE,
        ),
        'solr_field_markup' => array('#markup' => filter_xss($field['solr_field'])),
        'display_label' => array(
          '#type' => 'textfield',
          '#default_value' => isset($field['display_label']) ? filter_xss($field['display_label']) : '',
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
      '',
    ),
    'rows' => $rows,
    '#theme' => 'islandora_solr_metadata_management_table',
    '#empty' => t('No fields associated'),
  );
  return $form;
}