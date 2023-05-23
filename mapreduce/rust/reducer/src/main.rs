use std::{collections::HashMap, io::Write};

fn main() {
    let mut wtc: HashMap<String, usize> = HashMap::new();

    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin()
        .lines()
        .filter_map(|line| line.ok())
        .map(|line| {
            let split = line.split_once(',').unwrap();
            (split.0.to_owned(), split.1.to_owned())
        })
        .map(|(word, str_count)| {
            (word, str_count.parse::<usize>().unwrap_or(0)) 
        })
        .for_each(|(word, count)| {
            if let Some(crt_count) = wtc.get_mut(&word) {
                *crt_count += count; 
            } else {
                wtc.insert(word, count);
            }
        });

    wtc.iter().for_each(|(word, count)| { stdout.write_fmt(format_args!("{word},{count}\n")); });
}
