package nl.pointless.webmail.web.component;

import nl.pointless.commons.web.behavior.FocusBehavior;

import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.ajax.markup.html.form.AjaxButton;
import org.apache.wicket.extensions.ajax.markup.html.modal.ModalWindow;
import org.apache.wicket.markup.ComponentTag;
import org.apache.wicket.markup.html.WebPage;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.TextField;
import org.apache.wicket.markup.html.resources.StyleSheetReference;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;

/**
 * The search dialog page.
 * 
 * @author Peter Postma
 */
public class SearchDialogPage extends WebPage {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructor.
	 * 
	 * @param modalWindow The {@link ModalWindow}.
	 * @param messageDataProvider Message list data provider.
	 */
	public SearchDialogPage(final ModalWindow modalWindow,
			final MessageListDataProvider messageDataProvider) {
		Label pageTitleLabel = new Label("pageTitleId", new ResourceModel(
				"title.search"));
		add(pageTitleLabel);

		add(new StyleSheetReference("cssId", SearchDialogPage.class,
				"css/search-dialog.css"));

		final AjaxButton searchSubmitButton = new AjaxButton("submitButtonId",
				new ResourceModel("button.search")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onSubmit(AjaxRequestTarget target, Form<?> form) {
				modalWindow.close(target);
			}
		};

		Form<Void> form = new Form<Void>("formId") {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onComponentTag(ComponentTag tag) {
				super.onComponentTag(tag);

				tag.put("onsubmit", "document.getElementById('"
						+ searchSubmitButton.getMarkupId()
						+ "').click(); return false;");
			}
		};
		add(form);

		TextField<String> searchTextField = new TextField<String>(
				"searchTextFieldId", new PropertyModel<String>(
						messageDataProvider, "filter"));
		searchTextField.add(new FocusBehavior());
		form.add(searchTextField);

		form.add(searchSubmitButton);
	}
}
