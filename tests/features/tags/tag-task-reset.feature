Feature: Tag reset
  As someone using tasks and notes
  I can reset a task from all it's tags
  In order to maintain the semantic

  Scenario: Resetting a task dissociate it from all its tags
    Given I display the "Tags / Physics" page
    And there is an item named "Buy a cake" in the central list
    When I drop the item on "Inbox" in the page list
    And I list the items
    And the list does not contain "Buy a cake"
    And I display the "Inbox" page
    And I look at the central list
    And I list the items
    Then the list is:
        | display                                                  |
        |"Capital in the Twenty-First Century" by Thomas Piketty   |
        | Buy apples                                               |
        | Buy a cake                                               |
        | Buy cheese                                               |
        | Buy pears                                                |
        | Create Sozi SVG                                          |
        | Errands                                                  |

