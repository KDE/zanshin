Feature: Inbox content
  As someone collecting notes
  I can display the Inbox
  In order to see the notes (e.g. any note not associated to tag)

  Scenario: Unorganized notes appear in the inbox
    Given I display the "Inbox" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                                       |
       | 21/04/2014 14:49                              |
