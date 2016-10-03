Feature: Inbox content
  As someone collecting tasks
  I can display the Inbox
  In order to see the tasks which need to be organized (e.g. any task not associated to any project or context)

  Scenario: Unorganized tasks appear in the inbox
    Given I display the "Inbox" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                                                 |
       | Errands                                                 |
       | "Capital in the Twenty-First Century" by Thomas Piketty |
       | "The Pragmatic Programmer" by Hunt and Thomas           |
       | Buy cheese                                              |
       | Buy kiwis                                               |
       | Buy apples                                              |
       | Buy pears                                               |
       | Buy rutabagas                                           |
