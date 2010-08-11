package nl.pointless.webmail.web.component;

import nl.pointless.commons.web.component.AbstractAjaxImageButton;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.model.IModel;

/**
 * Like {@link AbstractActionButton}, but AJAX enabled.
 * 
 * @author Peter Postma
 */
public abstract class AbstractAjaxActionButton extends Panel {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructs an {@link AbstractAjaxActionButton}.
	 * 
	 * @param id Panel id.
	 * @param imageName Name of the image.
	 * @param labelModel Label model.
	 */
	public AbstractAjaxActionButton(String id, String imageName,
			IModel<String> labelModel) {
		super(id);

		ImageButton imageButton = new AbstractAjaxImageButton("buttonId",
				new ResourceReference(AbstractActionButton.class, imageName)) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onSubmit(AjaxRequestTarget target) {
				AbstractAjaxActionButton.this.onClick(target);
			}
		};
		add(imageButton);

		Label refreshLabel = new Label("labelId", labelModel);
		add(refreshLabel);

		setRenderBodyOnly(true);
	}

	/**
	 * Action when the button is clicked.
	 * 
	 * @param target An {@link AjaxRequestTarget}.
	 */
	protected abstract void onClick(AjaxRequestTarget target);
}
