Feature: Inbox content
  As someone collecting tasks and notes
  I can display the Inbox
  In order to see the artifacts which need to be organized (e.g. any task or note not associated to any project, context or tag)

  Scenario: Unorganized tasks and notes appear in the inbox
    Given I display the "Inbox" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy apples                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |
