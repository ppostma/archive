package nl.pointless.webmail.web.component;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.model.IModel;

/**
 * An {@link AbstractActionButton} contains a clickable image with a label.
 * 
 * @author Peter Postma
 */
public abstract class AbstractActionButton extends Panel {

	private static final long serialVersionUID = -6830949644718177877L;

	/**
	 * Constructs an {@link AbstractActionButton}.
	 * 
	 * @param id Panel id.
	 * @param imageName Name of the image.
	 * @param labelModel Label model.
	 */
	public AbstractActionButton(String id, String imageName,
			IModel<String> labelModel) {
		super(id);

		ImageButton refreshButton = new ImageButton("buttonId",
				new ResourceReference(AbstractActionButton.class, imageName)) {

			private static final long serialVersionUID = 1L;

			@Override
			public void onSubmit() {
				AbstractActionButton.this.onClick();
			}
		};
		add(refreshButton);

		Label refreshLabel = new Label("labelId", labelModel);
		add(refreshLabel);

		setRenderBodyOnly(true);
	}

	/**
	 * Action when the button is clicked.
	 */
	protected abstract void onClick();
}
