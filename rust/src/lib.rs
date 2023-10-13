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

pub struct Snippet {
    title: String,
    contents: String,
    absolute_path: String,
    tags: Vec<String>,
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

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    fn test_snippet_filename(name: &str) -> String {
        let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        path = path.join("test_data").join(name);
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
}
