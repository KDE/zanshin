Feature: Tag note association
  As someone collecting notes
  I can associate notes to a tag
  In order to describe the notes

  Scenario: Dropping a notes on a tag
    Given I display the "Inbox" page
    And there is an item named "A note about nothing interesting" in the central list
    When I drop the item on "Tags / Physics" in the page list
    And I display the "Tags / Physics" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                          |
       | A note about physics             |
       | A note about nothing interesting |

