Feature: Note creation from a tag
    As someone collecting notes
    I can add a note directly associated to a tag
    In order to give my note some semantic

  Scenario: Note added from a tag appear in its list
    Given I display the "Tags / Physics" page
    When I add a "note" named "Studies in fluid mechanics"
    And I look at the central list
    When I list the items
    Then the list is:
       | display                          |
       | A note about physics             |
       | Studies in fluid mechanics       |
