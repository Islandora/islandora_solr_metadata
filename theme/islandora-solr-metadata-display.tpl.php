<fieldset class="collapsible collapsed">
  <legend><span class="fieldset-legend"><?php print t('Details'); ?></span></legend>
  <div class="fieldset-wrapper">
    <dl xmlns:dcterms="http://purl.org/dc/terms/" class="islandora-inline-metadata islandora-metadata-fields">
      <?php $row_field = 0; ?>
      <?php foreach($solr_fields as $value): ?>
        <dt class="<?php print $row_field == 0 ? ' first' : ''; ?>">
          <?php print $value['display_label']; ?>
        </dt>
        <dd class="<?php print $row_field == 0 ? ' first' : ''; ?>">
          <?php print implode('<br/>', $value['value']); ?>
        </dd>
        <?php $row_field++; ?>
      <?php endforeach; ?>
    </dl>
  </div>
</fieldset>
