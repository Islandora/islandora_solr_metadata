<?php

function islandora_solr_metadata_delete_config_form($form, &$form_state, $configuration_id) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  $form_state['configuration_id'] = $configuration_id;
  $configuration_name = islandora_solr_metadata_retrieve_configuration_name($configuration_id);
  $form_state['configuration_name'] = $configuration_name;
  return confirm_form($form,
    t('Are you sure you want to delete the @configuration_name display configuration?', array('@configuration_name' => $configuration_name)),
    "admin/islandora/search/islandora_solr_metadata/config/$configuration_id",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function islandora_solr_metadata_delete_config_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  islandora_solr_metadata_delete_configuration($form_state['configuration_id']);
  $form_state['redirect'] = 'admin/islandora/search/islandora_solr/metadata';
  drupal_set_message(t('The @configuration_name display configuration has been deleted!', array('@configuration_name' => $form_state['configuration_name'])));
}