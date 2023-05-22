use std::io::Write;

fn main() {
    let mut stdout = std::io::BufWriter::new(std::io::stdout());
    std::io::stdin().lines().for_each(|line| {
        line.unwrap().split_whitespace().for_each(|word| {
            stdout.write_fmt(format_args!("{word},1\n"));
        });
    });
}
