use std::cmp::Ordering;
use std::cmp::Ordering::{Equal, Greater, Less};

/// Coordinates of a cell in the terminal screen.
/// First argument is the line, second argument is the column
#[derive(Copy, Clone)]
pub struct CellCoordinates(pub u16, pub u16);

impl PartialEq<CellCoordinates> for CellCoordinates {
    fn eq(&self, other: &CellCoordinates) -> bool {
        self.0 == other.0 && self.1 == other.1
    }
}

impl PartialOrd<CellCoordinates> for CellCoordinates {
    fn partial_cmp(&self, other: &CellCoordinates) -> Option<Ordering> {
        match self.0.partial_cmp(&other.0) {
            None => None,
            Some(Less) => Some(Less),
            Some(Equal) => self.1.partial_cmp(&other.1),
            Some(Greater) => Some(Greater),
        }
    }
}
