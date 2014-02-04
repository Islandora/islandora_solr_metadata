<?php

/**
 * @file
 * Form for general module configuration.
 */

/**
 * Form building function for general config.
 */
function islandora_solr_metadata_general_admin_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_solr_metadata', 'includes/general.admin');

  $form += array(
    'islandora_solr_metadata_omit_empty_values' => array(
      '#type' => 'checkbox',
      '#title' => t('Omit Empty Values'),
      '#description' => t('Avoid rendering fields which have no values associated with them.'),
      '#default_value' => variable_get('islandora_solr_metadata_omit_empty_values', FALSE),
    ),
    'actions' => array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Submit'),
      ),
    ),
  );

  return $form;
}

/**
 * Submission handler for general config form.
 */
function islandora_solr_metadata_general_admin_form_submit(&$form, &$form_state) {
  $variables = array(
    'islandora_solr_metadata_omit_empty_values',
  );

  foreach ($variables as $variable) {
    variable_set($variable, $form_state['values'][$variable]);
  }
}
