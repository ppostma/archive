package nl.pointless.webmail.web.component;

import nl.pointless.commons.web.component.AbstractAjaxImageButton;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.model.IModel;

/**
 * Like {@link BasicActionButton}, but AJAX enabled.
 * 
 * @author Peter Postma
 */
public abstract class AjaxActionButton extends
		AbstractActionButton {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructs an {@link AjaxActionButton}.
	 * 
	 * @param id Panel id.
	 * @param imageName Name of the image.
	 * @param labelModel Label model.
	 */
	public AjaxActionButton(String id, String imageName,
			IModel<String> labelModel) {
		super(id, imageName, labelModel);
	}

	@Override
	protected ImageButton createImageButton(String id, String imageName) {
		ImageButton imageButton = new AbstractAjaxImageButton(id,
				new ResourceReference(BasicActionButton.class, imageName)) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onSubmit(AjaxRequestTarget target) {
				AjaxActionButton.this.onClick(target);
			}
		};
		return imageButton;
	}

	/**
	 * Action when the button is clicked.
	 * 
	 * @param target An {@link AjaxRequestTarget}.
	 */
	protected abstract void onClick(AjaxRequestTarget target);
}
