package nl.pointless.webmail.web.component;

import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Button;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.model.IModel;

/**
 * An {@link AbstractActionButton} contains a clickable image with a label.
 * 
 * @author Peter Postma
 */
public abstract class AbstractActionButton extends Panel {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructs an {@link AbstractActionButton}.
	 * 
	 * @param id Panel id.
	 * @param imageUrl Url of the image.
	 * @param labelModel Label model.
	 */
	public AbstractActionButton(String id, String imageUrl,
			IModel<String> labelModel) {
		super(id);

		Button imageButton = createButton("buttonId", imageUrl);
		add(imageButton);

		Label refreshLabel = new Label("labelId", labelModel);
		add(refreshLabel);

		setRenderBodyOnly(true);
	}

	/**
	 * Creates the Button.
	 * 
	 * @param id Wicket id.
	 * @param imageUrl Url of the image.
	 * @return a {@link Button}
	 */
	protected abstract Button createButton(String id, String imageUrl);
}
