/*
  Copyright (c) 2023 Sergio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

use std::fs;
use std::fs::File;
use std::io::{self, BufRead, BufReader};

/// Represents a single snippet
/// Allows to load and save a single snippet from disk
pub struct Snippet {
    title: String,
    contents: String,
    absolute_path: String,
    tags: Vec<String>,
}

/// Represents a group of snippets
/// Allows to search for snippets and load them
pub struct SnippetFolder {
    root_path: String,
    snippets: Vec<Snippet>,
    sub_folders: Vec<SnippetFolder>,
}

impl Snippet {
    pub fn new(path: &str) -> Snippet {
        Snippet {
            title: String::new(),
            contents: String::new(),
            absolute_path: String::from(path),
            tags: Vec::new(),
        }
    }

    pub fn load_from_file(path: &str) -> io::Result<Snippet> {
        let mut snippet = Snippet::new(path);

        let file = File::open(path)?;
        let reader = BufReader::new(file);
        let mut lines = reader.lines();

        if let Some(Ok(line1)) = lines.next() {
            snippet.title = line1;
            if let Some(Ok(tags_str)) = lines.next() {
                snippet.tags = tags_str
                    .split(';')
                    .map(String::from)
                    .filter(|s| !s.is_empty())
                    .collect();
                let rest: Vec<String> = lines.filter_map(|line| line.ok()).collect();
                snippet.contents = rest.join("\n");
            }
        }

        let contents = fs::read_to_string(&snippet.absolute_path);
        contents?;

        Ok(snippet)
    }

    pub fn is_valid(&self) -> bool {
        !self.contents.is_empty()
    }
}

impl SnippetFolder {
    pub fn new(root_path: &str) -> SnippetFolder {
        SnippetFolder {
            root_path: String::from(root_path),
            snippets: vec![],
            sub_folders: vec![],
        }
    }

    /// Iteratively recurs and fills in the list of names, but does not load the snippet's contents
    pub fn load_names(&mut self) -> io::Result<()> {
        let dir = fs::read_dir(&self.root_path)?;
        for entry in dir {
            let entry = entry?;
            let path = entry.path();
            let path_str = path.to_str().unwrap();
            if path.is_dir() {
                let mut sub_folder = SnippetFolder::new(path_str);
                sub_folder.load_names()?;
                self.sub_folders.push(sub_folder);
            } else if let Some(ext) = path.extension() {
                if ext == "snip" {
                    let snippet = Snippet::new(path_str);
                    self.snippets.push(snippet);
                }
            }
        }

        Ok(())
    }

    /// Returns the file paths of all snippets
    /// Just for debugging/tests purposes
    pub fn list_names_recursive(&self) -> Vec<&str> {
        let mut names: Vec<&str> = vec![];
        for snippet in &self.snippets {
            names.push(snippet.absolute_path.as_str());
        }

        for sub_folder in &self.sub_folders {
            let sub_names = sub_folder.list_names_recursive();
            names.extend(sub_names);
        }

        names.sort();
        return names;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::{Path, PathBuf};

    fn test_snippets_root() -> String {
        let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        path = path.join("test_data");
        return path.to_str().unwrap().to_string();
    }

    fn test_snippet_filename(name: &str) -> String {
        let path = PathBuf::from(test_snippets_root()).join(name);
        return path.to_str().unwrap().to_string();
    }

    #[test]
    fn tst_read_snippet() {
        let filename = test_snippet_filename("1.snip");
        let empty_snippet = Snippet::new(&filename);

        assert_eq!(filename, empty_snippet.absolute_path);
        assert!(!empty_snippet.is_valid());

        let snippet = Snippet::load_from_file(&filename).expect("Failed to load from file");
        assert!(snippet.is_valid());

        assert_eq!(snippet.title, "mytitle");
        assert_eq!(snippet.tags, ["a", "b", "c", "d"]);
        assert_eq!(snippet.contents, "contents\ncontents\ncontents");
    }

    #[test]
    fn tst_list_snippets() {
        let mut root = SnippetFolder::new(test_snippets_root().as_str());
        assert_eq!(root.root_path, test_snippets_root());
        let _ = root.load_names();
        assert_eq!(root.snippets.len(), 3);
        assert_eq!(root.sub_folders.len(), 3);
        let names = root.list_names_recursive();
        for name in &names {
            println!("{}", name);
        }

        let expected_names = vec![
            "1.snip",
            "2.snip",
            "3.snip",
            "sub1/1.1.snip",
            "sub1/1.2.snip",
            "sub1/1.3.snip",
            "sub1/sub11/1.1.1.snip",
            "sub2/2.1.snip",
            "sub2/2.2.snip",
            "sub2/2.3.snip",
            "sub3/3.1.snip",
            "sub3/3.2.snip",
        ];

        // Remove the path prefix
        let stripped_names: Vec<&str> = names
            .iter()
            .map(|n| {
                Path::new(*n)
                    .strip_prefix(test_snippets_root())
                    .unwrap()
                    .to_str()
                    .unwrap()
            })
            .collect();

        assert_eq!(stripped_names, expected_names);
    }
}
