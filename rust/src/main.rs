use snippyrs::SnippetFolder;
use std::{env, io};

fn main() -> io::Result<()> {
    let root_path = env::var("SNIPPY_FOLDER").expect("SNIPPY_FOLDER not set!");
    let mut root = SnippetFolder::new(root_path.as_str());
    root.load_names()?;
    for name in root.list_names_recursive() {
        println!("{}", name);
    }

    Ok(())
}
