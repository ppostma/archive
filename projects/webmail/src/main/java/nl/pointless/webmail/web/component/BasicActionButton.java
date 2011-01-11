package nl.pointless.webmail.web.component;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.model.IModel;

/**
 * A {@link BasicActionButton} contains a clickable image with a label.
 * 
 * @author Peter Postma
 */
public abstract class BasicActionButton extends AbstractActionButton {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructs an {@link BasicActionButton}.
	 * 
	 * @param id Panel id.
	 * @param imageName Name of the image.
	 * @param labelModel Label model.
	 */
	public BasicActionButton(String id, String imageName,
			IModel<String> labelModel) {
		super(id, imageName, labelModel);
	}

	@Override
	protected ImageButton createImageButton(String id, String imageName) {
		ImageButton imageButton = new ImageButton(id, new ResourceReference(
				BasicActionButton.class, imageName)) {

			private static final long serialVersionUID = 1L;

			@Override
			public void onSubmit() {
				BasicActionButton.this.onClick();
			}
		};
		return imageButton;
	}

	/**
	 * Action when the button is clicked.
	 */
	protected abstract void onClick();
}
