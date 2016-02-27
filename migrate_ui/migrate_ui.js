(function ($) {

/**
 * Provide the summary information for the migration detail vertical tabs.
 */
Backdrop.behaviors.migrateUISummary = {
  attach: function (context) {
    // The backdropSetSummary method required for this behavior is not available
    // on the Blocks administration page, so we need to make sure this
    // behavior is processed only if setSummary is defined.
    if (typeof jQuery.fn.backdropSetSummary == 'undefined') {
      return;
    }

    $('fieldset#edit-overview', context).backdropSetSummary(function (context) {
      if (!$('#owner', context).children()) {
        return '<span class="error">' + Backdrop.t('Missing client owner.') + '</span>';
      }
    });
    $('fieldset#edit-destination', context).backdropSetSummary(function (context) {
      total = $('tr', context).length - 2;
      unmapped = $('td.migrate-error', context).length / 2;
      mapped = total - unmapped;
      msg = Backdrop.formatPlural(mapped, '1 mapping.', '@count mapped.');
      if (unmapped) {
        msg = '<span class="error">' + Backdrop.formatPlural(unmapped, '1 unmapped', '@count unmapped') + '</span>' + '. ' + msg;
      }
      return msg;
    });
    $('fieldset#edit-source', context).backdropSetSummary(function (context) {
      total = $('tr', context).length - 2;
      unmapped = $('td.migrate-error', context).length / 2;
      mapped = total - unmapped;
      msg = Backdrop.formatPlural(mapped, '1 mapping.', '@count mapped.');
      if (unmapped) {
        msg = '<span class="error">' + Backdrop.formatPlural(unmapped, '1 unmapped', '@count unmapped') + '</span>' + '. ' + msg;
      }
      return msg;
    });

    $('fieldset.migrate-mapping').each(function ($context) {
      msg = Backdrop.t('By priority: ');
      var levels= {1:'OK',2:'Low',3:'Medium',4:'Blocker'};
      for (level in levels) {
        txt = '';
        if (count = $(this).find('td.migrate-priority-' + level).length / 5) {
          txt = count + ' ' + levels[level];
          if (level > 1) {
            txt = '<span class="error">' + txt  + '</span>';
          }
          msg = msg + txt + '. ';
        }
      }
      $(this).backdropSetSummary(msg);
    }
  )}
}

})(jQuery);
