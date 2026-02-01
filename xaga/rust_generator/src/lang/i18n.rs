use anyhow::Result;
use fluent_bundle::{FluentBundle, FluentResource};
use std::cell::RefCell;
use std::rc::Rc;
use unic_langid::LanguageIdentifier;

thread_local! {
    static I18N: RefCell<Option<Rc<FluentBundle<FluentResource>>>> = RefCell::new(None);
}

pub fn init_translate(ftl: String, lang: &str) -> Result<()> {
    let langid: LanguageIdentifier = lang.parse()?;
    let res = FluentResource::try_new(ftl).map_err(|(_, e)| e[0].clone())?;
    let mut bundle = FluentBundle::new(vec![langid]);
    bundle
        .add_resource(res)
        .map_err(|errs| anyhow::anyhow!("Fluent errors: {:?}", errs))?;

    I18N.with(|cell| *cell.borrow_mut() = Some(Rc::new(bundle)));
    Ok(())
}

pub fn translate_string(key: &str) -> String {
    I18N.with(|cell| {
        let borrowed = cell.borrow();
        let bundle = borrowed.as_ref().unwrap();
        let msg = match bundle.get_message(key) {
            Some(m) => m,
            None => return key.to_string(),
        };
        let pattern = match msg.value() {
            Some(p) => p,
            None => return key.to_string(),
        };
        let mut errors: Vec<fluent::FluentError> = vec![];
        bundle.format_pattern(pattern, None, &mut errors).to_string()
    })
}
