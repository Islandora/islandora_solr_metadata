<?php

function islandora_solr_metadata_admin_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $associations = islandora_solr_metadata_get_associations();
  $form = array();
  $rows = array();
  foreach ($associations as $association) {
    $cmodels = islandora_solr_metadata_get_cmodels($association['id']);
    $rows[] = array(
      l($association['name'], "admin/islandora/search/islandora_solr_metadata/config/{$association['id']}"),
      empty($cmodels) ? t('No content models currently associated') : theme('item_list', array(
        'items' => array_keys($cmodels),
      ))
    );
  }
  $form['table'] = array(
    '#title' => t('Solr metadata associations'),
    '#markup' => theme('table', array(
      'header' => array(
        t('Name'),
        t('Associated content models'),
      ),
      'rows' => $rows,
      'empty' => t('No associations currently present.'),
    ))
  );
  $form['add_configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['add_configuration']['configuration_name'] = array(
    '#type' => 'textfield',
    '#size' => 100,
    '#title' => t('Configuration name'),
  );
  $form['add_configuration']['save_content_model'] = array(
    '#type' => 'submit',
    '#value' => t('Add configuration'),
    '#name' => 'islandora_solr_metadata_add_configuration',
  );
  return $form;
}

function islandora_solr_metadata_admin_form_validate($form, $form_state) {
  if (empty($form_state['values']['configuration_name'])) {
    form_set_error('configuration_name', t('Please enter a non-empty configuration name!'));
  }
}

function islandora_solr_metadata_admin_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  islandora_solr_metadata_add_configuration($form_state['values']['configuration_name']);
  drupal_set_message(t('A new empty configuration has been created for @config_name', array('@config_name' => $form_state['values']['configuration_name'])));
}