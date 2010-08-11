package nl.pointless.commons.web.component;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.ajax.form.AjaxFormSubmitBehavior;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.util.string.AppendingStringBuffer;

/**
 * AJAX enabled {@link ImageButton}.
 * 
 * @author Peter Postma
 */
public abstract class AbstractAjaxImageButton extends ImageButton {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructor.
	 * 
	 * @param id Component id.
	 * @param resourceReference A {@link ResourceReference}.
	 */
	public AbstractAjaxImageButton(String id,
			ResourceReference resourceReference) {
		super(id, resourceReference);

		add(new AjaxFormSubmitBehavior(null, "onclick") {

			private static final long serialVersionUID = 1L;

			/**
			 * @see org.apache.wicket.ajax.form.AjaxFormSubmitBehavior#onSubmit(org.apache.wicket.ajax.AjaxRequestTarget)
			 */
			@Override
			protected void onSubmit(AjaxRequestTarget target) {
				AbstractAjaxImageButton.this.onSubmit(target);
			}

			/**
			 * @see org.apache.wicket.ajax.form.AjaxFormSubmitBehavior#onError(org.apache.wicket.ajax.AjaxRequestTarget)
			 */
			@Override
			protected void onError(AjaxRequestTarget target) {
				AbstractAjaxImageButton.this.onError(target);
			}

			/**
			 * @see org.apache.wicket.ajax.form.AjaxFormSubmitBehavior#getEventHandler()
			 */
			@Override
			protected CharSequence getEventHandler() {
				AppendingStringBuffer handler = new AppendingStringBuffer();
				handler.append(super.getEventHandler());
				handler.append("; return false;");
				return handler;
			}
		});
	}

	/**
	 * Action when the button is clicked.
	 * 
	 * @param target An {@link AjaxRequestTarget}.
	 */
	protected abstract void onSubmit(AjaxRequestTarget target);

	/**
	 * Listener method invoked on form submit with errors
	 * 
	 * @param target An {@link AjaxRequestTarget}.
	 */
	protected void onError(AjaxRequestTarget target) {
		// No implementation
	}
}
