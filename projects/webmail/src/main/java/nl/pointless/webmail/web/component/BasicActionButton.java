package nl.pointless.webmail.web.component;

import org.apache.wicket.markup.ComponentTag;
import org.apache.wicket.markup.html.form.Button;
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
	 * @param imageUrl Url of the image.
	 * @param labelModel Label model.
	 */
	public BasicActionButton(String id, String imageUrl,
			IModel<String> labelModel) {
		super(id, imageUrl, labelModel);
	}

	@Override
	protected Button createButton(final String id, final String imageUrl) {
		Button imageButton = new Button(id) {

			private static final long serialVersionUID = 1L;

			@Override
			public void onSubmit() {
				BasicActionButton.this.onClick();
			}

			@Override
			protected void onComponentTag(ComponentTag tag) {
				super.onComponentTag(tag);

				checkComponentTag(tag, "input");
				checkComponentTagAttribute(tag, "type", "image");

				tag.put("src", imageUrl);
			}
		};

		return imageButton;
	}

	/**
	 * Action when the button is clicked.
	 */
	protected abstract void onClick();
}
